// Stufe 5 – An das Gerät anpassen: Geräteabfragen, Specialization Constants,
// Fehlerbehandlung
//
// Problem der Stufe 4: Die Kachelgröße T ist beim Übersetzen fest. Die beste
// Wahl hängt aber vom Gerät ab, und ein zu großes T (z. B. 64 -> 4096
// Work-Items je Work-Group) startet auf der GPU gar nicht, das Programm
// stürzt ab.
//
// Idee: T wird erst zur Laufzeit festgelegt, und zwar
//   1. über eine Specialization Constant: Der Host setzt T vor dem Start,
//      der JIT-Compiler setzt den Wert dann als Konstante in den Kernel ein.
//      Ein Quellcode, eine ausführbare Datei, trotzdem ein konstantes T.
//   2. nach einer Geräteabfrage: Kachelgrößen, die das Gerät nicht erlaubt,
//      werden gar nicht erst versucht.
//   3. nach einer kurzen Messung der übrigen Kandidaten (je drei Läufe).
// Fehler werden abgefangen (synchron per try/catch, asynchron über einen
// Async Handler), statt das Programm zu beenden.
//
// Kernel und Speicherverwaltung sonst wie in Stufe 4.
//
// Neue Konzepte: Geräteabfragen, specialization_id, kernel_handler,
// sycl::exception, Async Handler, wait_and_throw

#include <sycl/sycl.hpp>
#include <exception>
#include "common.hpp"

#ifndef __ACPP__
// Specialization Constant für die Kachelgröße (SYCL 2020, Vorgabe 16)
constexpr sycl::specialization_id<std::size_t> tile_id{16};
#endif

// Kandidaten für die Kachelgröße
constexpr std::size_t kandidaten[] = {8, 16, 32, 64};

// Kernel aus Stufe 4. Die Kachelgröße kommt als Argument für den Host
// (Größe von nd_range und lokalem Speicher) und als Specialization Constant
// für den Kernel.
sycl::event matmul_tiled(sycl::queue& q, const float* A, const float* B, float* C,
                         std::size_t n, std::size_t tile) {
  const std::size_t np = (n + tile - 1) / tile * tile;
#ifdef __ACPP__
  // AdaptiveCpp kennt specialization_id nicht, bietet aber eine eigene
  // Erweiterung mit derselben Wirkung
  const sycl::specialized<std::size_t> tile_spez{tile};
#endif
  return q.submit([&](sycl::handler& h) {
#ifndef __ACPP__
    h.set_specialization_constant<tile_id>(tile);
#endif
    sycl::local_accessor<float, 2> tileA({tile, tile}, h);
    sycl::local_accessor<float, 2> tileB({tile, tile}, h);

    h.parallel_for(
        sycl::nd_range<2>{{np, np}, {tile, tile}},
#ifdef __ACPP__
        [=](sycl::nd_item<2> it) {
          const std::size_t T = tile_spez;
#else
        [=](sycl::nd_item<2> it, sycl::kernel_handler kh) {
          // Für den JIT-Compiler eine Konstante
          const std::size_t T = kh.get_specialization_constant<tile_id>();
#endif
          const std::size_t i = it.get_global_id(0);
          const std::size_t j = it.get_global_id(1);
          const std::size_t li = it.get_local_id(0);
          const std::size_t lj = it.get_local_id(1);

          float sum = 0.0f;
          for (std::size_t t = 0; t < n; t += T) {
            tileA[li][lj] = (i < n && t + lj < n) ? A[i * n + t + lj] : 0.0f;
            tileB[li][lj] = (t + li < n && j < n) ? B[(t + li) * n + j] : 0.0f;
            sycl::group_barrier(it.get_group());

            for (std::size_t k = 0; k < T; ++k)
              sum += tileA[li][k] * tileB[k][lj];
            sycl::group_barrier(it.get_group());
          }
          if (i < n && j < n) C[i * n + j] = sum;
        });
  });
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);

  // Async Handler: Fehler, die erst während der Ausführung auf dem Gerät
  // auftreten, sammelt die Laufzeit und übergibt sie hier, sobald der Host
  // mit wait_and_throw() wartet. Wir geben sie als Ausnahme weiter, damit sie
  // genauso behandelt werden wie synchrone Fehler.
  auto async_handler = [](sycl::exception_list fehler) {
    for (const std::exception_ptr& e : fehler) std::rethrow_exception(e);
  };

  try {
    sycl::queue q{sycl::default_selector_v, async_handler};
    const sycl::device d = q.get_device();
    const std::string dev = d.get_info<sycl::info::device::name>();

    // 1) Geräteabfragen: Grenzen des gewählten Geräts
    const std::size_t max_wg = d.get_info<sycl::info::device::max_work_group_size>();
    const sycl::range<2> max_items =
        d.get_info<sycl::info::device::max_work_item_sizes<2>>();
    const std::size_t local_mem = d.get_info<sycl::info::device::local_mem_size>();
    std::fprintf(stderr,
                 "[stufe5] %s: max. Work-Group %zu, max. je Dimension %zu x %zu, "
                 "lokaler Speicher %zu Byte\n",
                 dev.c_str(), max_wg, max_items[0], max_items[1], local_mem);

    std::vector<float> hA(n * n), hB(n * n), hC(n * n);
    bench::fill_random(hA.data(), n * n, 1);
    bench::fill_random(hB.data(), n * n, 2);

    float* A = sycl::malloc_device<float>(n * n, q);
    float* B = sycl::malloc_device<float>(n * n, q);
    float* C = sycl::malloc_device<float>(n * n, q);

    auto copy_in = [&] {
      q.memcpy(A, hA.data(), bytes);
      q.memcpy(B, hB.data(), bytes);
      q.wait_and_throw();
    };
    auto copy_out = [&] {
      q.memcpy(hC.data(), C, bytes);
      q.wait_and_throw();
    };

    bench::Timer t0;
    copy_in();

    // 2) Auswahl: erst Kachelgrößen aussortieren, die das Gerät laut Abfrage
    //    nicht erlaubt, dann die übrigen kurz messen. Der erste Aufruf je
    //    Kachelgröße enthält die JIT-Übersetzung und wird nicht gewertet.
    std::size_t T = 0;
    double beste_ms = 0.0;
    for (const std::size_t t : kandidaten) {
      const bool passt = t * t <= max_wg && t <= max_items[0] && t <= max_items[1] &&
                         2 * t * t * sizeof(float) <= local_mem;
      if (!passt) {
        std::fprintf(stderr, "[stufe5] T = %zu: vom Gerät nicht erlaubt\n", t);
        continue;
      }
      try {
        matmul_tiled(q, A, B, C, n, t);
        q.wait_and_throw();
        double ms = 1e300;                    // schnellster von drei Läufen
        for (int r = 0; r < 3; ++r) {
          bench::Timer tv; matmul_tiled(q, A, B, C, n, t); q.wait_and_throw();
          ms = std::min(ms, tv.ms()); }
        std::fprintf(stderr, "[stufe5] T = %zu: %.3f ms\n", t, ms);
        if (T == 0 || ms < beste_ms) {
          T = t;
          beste_ms = ms;
        }
      } catch (const sycl::exception& e) {
        // Die Geräteabfrage liefert nur Obergrenzen. Scheitert der Kernel
        // trotzdem, wird die nächste Kachelgröße versucht.
        std::fprintf(stderr, "[stufe5] T = %zu: startet nicht (%s)\n", t, e.what());
      }
    }
    if (T == 0) {
      std::fprintf(stderr, "[stufe5] keine Kachelgröße läuft auf %s\n", dev.c_str());
      sycl::free(A, q);
      sycl::free(B, q);
      sycl::free(C, q);
      return 2;
    }
    std::fprintf(stderr, "[stufe5] gewählt: T = %zu (Auswahl %.1f ms)\n", T, t0.ms());

    auto kernel = [&] {
      matmul_tiled(q, A, B, C, n, T);
      q.wait_and_throw();
    };

    // Erster Lauf: hier einschließlich Abfrage und Auswahl
    kernel();
    copy_out();
    const double first = t0.ms();

    std::vector<double> kernel_times, transfer_times;
    for (int r = 0; r < cfg.reps; ++r) {
      bench::Timer tin;
      copy_in();
      const double in_ms = tin.ms();

      bench::Timer tk;
      kernel();
      kernel_times.push_back(tk.ms());

      bench::Timer tout;
      copy_out();
      transfer_times.push_back(in_ms + tout.ms());
    }

    const bool ok = bench::verify(hA.data(), hB.data(), hC.data(), n);
    bench::report("5_angepasst_T" + std::to_string(T), dev, n, first,
                  bench::median(kernel_times), bench::median(transfer_times), ok);

    sycl::free(A, q);
    sycl::free(B, q);
    sycl::free(C, q);
    return ok ? 0 : 1;
  } catch (const sycl::exception& e) {
    // 3) Alles, was außerhalb der Auswahl schiefgeht (kein Gerät, Speicher
    //    voll, Fehler im gewählten Kernel), beendet das Programm geordnet.
    std::fprintf(stderr, "[stufe5] SYCL-Fehler: %s\n", e.what());
    return 2;
  }
}
