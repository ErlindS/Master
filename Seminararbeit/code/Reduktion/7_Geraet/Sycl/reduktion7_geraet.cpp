// Reduktion Variante 7 – Gruppengröße und Zahl der Work-Items vom Gerät
//
// Gegenstück zu Stufe 5 der Matrixmultiplikation. Die Varianten 1 bis 3 setzen
// beide Parameter fest: 256 Work-Items je Gruppe und höchstens 2^20 Work-Items
// insgesamt (rahmen::global_size). Beides passt nicht zu jedem Gerät -- eine
// CPU mit sechs Kernen braucht völlig andere Werte als eine H100. Hier werden
// sie zur Laufzeit bestimmt:
//   1. Geräteabfragen liefern die erlaubte Gruppengröße und die Zahl der
//      Recheneinheiten; daraus entstehen die Kandidaten,
//   2. die Kandidaten werden kurz gemessen (je drei Läufe),
//   3. der schnellste wird für die eigentliche Messung verwendet.
// Fehler fängt ein try/catch mit Async Handler ab.
//
// Anders als beim Stencil braucht der Kernel hier keine Specialization
// Constant: Die Gruppengröße steht nur in nd_range, im Kernel selbst hängt
// keine Feldgröße von ihr ab. Das Kollektiv reduce_over_group arbeitet mit
// jeder Gruppengröße.
//
// Mit ./reduktion7_geraet <n> [wiederholungen] [W] lässt sich die Auswahl
// übergehen und ein festes W messen (zum Vergleich mit Variante 3).

#include <sycl/sycl.hpp>
#include <exception>
#include "sycl_rahmen.hpp"

constexpr std::size_t kandidaten[] = {32, 64, 128, 256, 512, 1024};

// Ein Reduktionslauf mit Gruppengröße W und G Work-Items insgesamt.
sycl::event summe(sycl::queue& q, const float* x, float* sum, std::size_t n, std::size_t W,
                  std::size_t G) {
  return q.parallel_for(sycl::nd_range<1>{G, W}, [=](sycl::nd_item<1> it) {
    float p = 0.0f;
    for (std::size_t k = it.get_global_id(0); k < n; k += G) p += x[k];
    const float g = sycl::reduce_over_group(it.get_group(), p, sycl::plus<float>());
    if (it.get_local_id(0) == 0) {
      sycl::atomic_ref<float, sycl::memory_order::relaxed, sycl::memory_scope::device,
                       sycl::access::address_space::global_space>
          r(*sum);
      r.fetch_add(g);
    }
  });
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t vorgabe = (argc > 3) ? std::strtoul(argv[3], nullptr, 10) : 0;

  auto async_handler = [](sycl::exception_list fehler) {
    for (const std::exception_ptr& e : fehler) std::rethrow_exception(e);
  };

  try {
    sycl::queue q{sycl::default_selector_v, async_handler};
    const sycl::device d = q.get_device();

    const std::size_t max_wg = d.get_info<sycl::info::device::max_work_group_size>();
    const std::size_t einheiten = d.get_info<sycl::info::device::max_compute_units>();
    std::fprintf(stderr, "[reduktion7] %s: max. Work-Group %zu, %zu Recheneinheiten\n",
                 d.get_info<sycl::info::device::name>().c_str(), max_wg, einheiten);

    // Zahl der Work-Items: so viele, dass jede Recheneinheit mehrere Gruppen
    // bekommt, aber nie mehr als Werte vorhanden sind. Jedes Work-Item summiert
    // n / G Werte privat auf.
    auto global = [&](std::size_t W) {
      const std::size_t wunsch = std::min<std::size_t>(n, einheiten * W * 8);
      return std::max<std::size_t>(W, (wunsch + W - 1) / W * W);
    };

    // Daten einmal auf das Gerät, danach messen wir nur den Kernel.
    const std::vector<float> hx = bench::reduktion::init(n);
    float* x = sycl::malloc_device<float>(n, q);
    float* sum = sycl::malloc_device<float>(1, q);
    if (!x || !sum) {
      std::fprintf(stderr, "[reduktion7] Gerätespeicher reicht nicht\n");
      return 2;
    }
    q.memcpy(x, hx.data(), n * sizeof(float)).wait_and_throw();

    std::size_t W = 0, G = 0;
    if (vorgabe > 0) {
      W = vorgabe;
      G = global(W);
      std::fprintf(stderr, "[reduktion7] W = %zu per Kommandozeile vorgegeben\n", W);
    } else {
      double beste_ms = 0.0;
      for (const std::size_t w : kandidaten) {
        if (w > max_wg) {
          std::fprintf(stderr, "[reduktion7] W = %zu: vom Gerät nicht erlaubt\n", w);
          continue;
        }
        const std::size_t g = global(w);
        try {
          q.memset(sum, 0, sizeof(float)).wait_and_throw();
          summe(q, x, sum, n, w, g).wait_and_throw();   // erster Lauf: JIT
          double ms = 1e300;
          for (int r = 0; r < 3; ++r) {
            q.memset(sum, 0, sizeof(float)).wait_and_throw();
            bench::Timer tv;
            summe(q, x, sum, n, w, g).wait_and_throw();
            ms = std::min(ms, tv.ms());
          }
          std::fprintf(stderr, "[reduktion7] W = %zu, %zu Work-Items: %.3f ms\n", w, g, ms);
          if (W == 0 || ms < beste_ms) {
            W = w;
            G = g;
            beste_ms = ms;
          }
        } catch (const sycl::exception& e) {
          std::fprintf(stderr, "[reduktion7] W = %zu: startet nicht (%s)\n", w, e.what());
        }
      }
    }

    sycl::free(x, q);
    sycl::free(sum, q);

    if (W == 0) {
      std::fprintf(stderr, "[reduktion7] keine Gruppengröße läuft auf diesem Gerät\n");
      return 2;
    }
    std::fprintf(stderr, "[reduktion7] gewählt: W = %zu, %zu Work-Items\n", W, G);

    // Eigentliche Messung über denselben Rahmen wie alle anderen Varianten.
    auto launch = [&](const float* xd, float* sd, std::size_t nn) {
      summe(q, xd, sd, nn, W, G);
    };
    return rahmen::reduktion("reduktion7_geraet_W" + std::to_string(W), q, cfg, launch);
  } catch (const sycl::exception& e) {
    std::fprintf(stderr, "[reduktion7] SYCL-Fehler: %s\n", e.what());
    return 2;
  }
}
