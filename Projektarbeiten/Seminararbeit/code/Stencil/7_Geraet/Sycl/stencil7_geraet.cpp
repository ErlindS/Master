// Stencil Variante 7 – Kachelgröße zur Laufzeit passend zum Gerät
//
// Gegenstück zu Stufe 5 der Matrixmultiplikation. Variante 3 (stencil3_lokal)
// hat die Kachelgröße B beim Übersetzen fest; die beste Wahl hängt aber vom
// Gerät ab, und eine zu große Kachel startet dort gar nicht. Hier wird B erst
// zur Laufzeit festgelegt:
//   1. Geräteabfragen sortieren aus, was das Gerät nicht erlaubt (Work-Group,
//      Work-Items je Dimension, lokaler Speicher für (B+2)^2 Werte),
//   2. die übrigen Kandidaten werden kurz gemessen (je drei Zeitschritte),
//   3. der Kernel bekommt B als Specialization Constant, damit der
//      JIT-Compiler es wie eine Konstante behandelt.
// Fehler werden abgefangen (try/catch und Async Handler) statt das Programm
// zu beenden: Die Abfrage liefert nur Obergrenzen, scheitern kann es trotzdem.
//
// Mit ./stencil7_geraet <n> [wiederholungen] [B] lässt sich die Auswahl
// übergehen und ein festes B messen (zum Vergleich mit Variante 3).
//
// Zwei Programme aus derselben Quelle, weil die Laufzeit-Kachelgröße an zwei
// Stellen wirken kann und sich die beiden Effekte nur getrennt erklären lassen:
//   stencil7_geraet        Halo-Laden ohne Division (zwei Schleifen)
//   stencil7_geraet_flach  Halo-Laden wie Variante 3 (-DLADEN_FLACH), also mit
//                          Division und Restwert durch B + 2
// Der Vergleich Variante 3 (festes B, flach) gegen stencil7_geraet_flach zeigt
// bei gleichem Code allein die Kosten der emulierten Specialization Constant;
// der Vergleich stencil7_geraet_flach gegen stencil7_geraet zeigt, wie viel
// davon auf die Ganzzahl-Division entfällt, die mit festem B wegoptimiert wird.
//
// Neue Konzepte gegenüber Variante 3: Geräteabfragen, specialization_id,
// kernel_handler, sycl::exception, Async Handler, wait_and_throw

#include <sycl/sycl.hpp>
#include <exception>
#include "sycl_rahmen.hpp"

#ifndef __ACPP__
constexpr sycl::specialization_id<std::size_t> block_id{16};
#endif

constexpr std::size_t kandidaten[] = {8, 16, 32};

// Ein Zeitschritt mit Halo-Kachel im lokalen Speicher, Kachelgröße als
// Argument (Host: nd_range und lokaler Speicher) und als Specialization
// Constant (Kernel).
sycl::event schritt(sycl::queue& q, const float* in, float* out, std::size_t n,
                    std::size_t B) {
  const std::size_t np = (n + B - 1) / B * B;
#ifdef __ACPP__
  const sycl::specialized<std::size_t> block_spez{B};
#endif
  return q.submit([&](sycl::handler& h) {
#ifndef __ACPP__
    h.set_specialization_constant<block_id>(B);
#endif
    sycl::local_accessor<float, 2> tile(sycl::range<2>{B + 2, B + 2}, h);
    const long N = static_cast<long>(n);
    h.parallel_for(sycl::nd_range<2>{{np, np}, {B, B}},
#ifdef __ACPP__
                   [=](sycl::nd_item<2> it) {
                     const long Bk = static_cast<long>(block_spez);
#else
                   [=](sycl::nd_item<2> it, sycl::kernel_handler kh) {
                     const long Bk =
                         static_cast<long>(kh.get_specialization_constant<block_id>());
#endif
                     const long li = it.get_local_id(0), lj = it.get_local_id(1);
                     const long i0 = long(it.get_group(0)) * Bk - 1;   // Ecke inkl. Halo
                     const long j0 = long(it.get_group(1)) * Bk - 1;

#ifdef LADEN_FLACH
                     // Flaches Laden wie in Variante 3: ein Index über alle
                     // (B+2)^2 Werte, aufgeteilt mit Division und Restwert.
                     // Mit festem B rechnet der Compiler beides in
                     // Multiplikation und Schieben um; als Specialization
                     // Constant auf dem CUDA-Backend bleibt es eine echte
                     // Ganzzahl-Division je geladenem Wert.
                     for (long k = li * Bk + lj; k < (Bk + 2) * (Bk + 2); k += Bk * Bk) {
                       const long ti = k / (Bk + 2), tj = k % (Bk + 2);
                       const long gi = i0 + ti, gj = j0 + tj;
                       tile[ti][tj] =
                           (gi >= 0 && gi < N && gj >= 0 && gj < N) ? in[gi * N + gj] : 0.0f;
                     }
#else
                     // Zwei verschachtelte Schleifen mit Schrittweite B: jedes
                     // Work-Item lädt ein bis vier Werte, ganz ohne Division.
                     // Deckt (B+2)^2 Werte genau einmal ab, weil li und lj
                     // kleiner als B sind.
                     for (long ti = li; ti < Bk + 2; ti += Bk)
                       for (long tj = lj; tj < Bk + 2; tj += Bk) {
                         const long gi = i0 + ti, gj = j0 + tj;
                         tile[ti][tj] =
                             (gi >= 0 && gi < N && gj >= 0 && gj < N) ? in[gi * N + gj] : 0.0f;
                       }
#endif
                     sycl::group_barrier(it.get_group());

                     const long i = i0 + 1 + li, j = j0 + 1 + lj;
                     if (i >= 1 && i < N - 1 && j >= 1 && j < N - 1)
                       out[i * N + j] =
                           bench::stencil::update(tile[li][lj + 1], tile[li + 2][lj + 1],
                                                  tile[li + 1][lj], tile[li + 1][lj + 2]);
                   });
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
    sycl::queue q{sycl::default_selector_v, async_handler,
                  sycl::property::queue::in_order{}};
    const sycl::device d = q.get_device();

    const std::size_t max_wg = d.get_info<sycl::info::device::max_work_group_size>();
    const sycl::range<2> max_items = d.get_info<sycl::info::device::max_work_item_sizes<2>>();
    const std::size_t local_mem = d.get_info<sycl::info::device::local_mem_size>();
    std::fprintf(stderr,
                 "[stencil7] %s: max. Work-Group %zu, max. je Dimension %zu x %zu, "
                 "lokaler Speicher %zu Byte\n",
                 d.get_info<sycl::info::device::name>().c_str(), max_wg, max_items[0],
                 max_items[1], local_mem);

    std::size_t B = 0;
    if (vorgabe > 0) {
      B = vorgabe;
      std::fprintf(stderr, "[stencil7] B = %zu per Kommandozeile vorgegeben\n", B);
    } else {
      // Auswahl auf zwei eigenen Puffern, damit der eigentliche Messrahmen
      // danach unberührt startet. Die Puffer werden vorher freigegeben.
      float* ta = sycl::malloc_device<float>(n * n, q);
      float* tb = sycl::malloc_device<float>(n * n, q);
      if (!ta || !tb) {
        std::fprintf(stderr, "[stencil7] Gerätespeicher reicht nicht\n");
        return 2;
      }
      q.memset(ta, 0, n * n * sizeof(float));
      q.memset(tb, 0, n * n * sizeof(float));
      q.wait_and_throw();

      double beste_ms = 0.0;
      for (const std::size_t b : kandidaten) {
        const bool passt = b * b <= max_wg && b <= max_items[0] && b <= max_items[1] &&
                           (b + 2) * (b + 2) * sizeof(float) <= local_mem;
        if (!passt) {
          std::fprintf(stderr, "[stencil7] B = %zu: vom Gerät nicht erlaubt\n", b);
          continue;
        }
        try {
          schritt(q, ta, tb, n, b);   // erster Lauf: enthält die JIT-Übersetzung
          q.wait_and_throw();
          double ms = 1e300;          // schnellster von drei Zeitschritten
          for (int r = 0; r < 3; ++r) {
            bench::Timer tv;
            schritt(q, ta, tb, n, b);
            q.wait_and_throw();
            ms = std::min(ms, tv.ms());
          }
          std::fprintf(stderr, "[stencil7] B = %zu: %.3f ms je Zeitschritt\n", b, ms);
          if (B == 0 || ms < beste_ms) {
            B = b;
            beste_ms = ms;
          }
        } catch (const sycl::exception& e) {
          std::fprintf(stderr, "[stencil7] B = %zu: startet nicht (%s)\n", b, e.what());
        }
      }
      sycl::free(ta, q);
      sycl::free(tb, q);
    }

    if (B == 0) {
      std::fprintf(stderr, "[stencil7] keine Kachelgröße läuft auf diesem Gerät\n");
      return 2;
    }
    std::fprintf(stderr, "[stencil7] gewählt: B = %zu\n", B);

    auto launch = [&](const float* in, float* out) { schritt(q, in, out, n, B); };
#ifdef LADEN_FLACH
    const std::string name = "stencil7_geraet_flach_B" + std::to_string(B);
#else
    const std::string name = "stencil7_geraet_B" + std::to_string(B);
#endif
    return rahmen::stencil(name, q, cfg, launch);
  } catch (const sycl::exception& e) {
    std::fprintf(stderr, "[stencil7] SYCL-Fehler: %s\n", e.what());
    return 2;
  }
}
