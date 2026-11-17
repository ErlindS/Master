// Reduktion Variante 1 – private Teilsummen, dann ein Atomic je Work-Item
//
// Jedes Work-Item summiert n / global Werte in einer Schleife privat auf
// und addiert seine Teilsumme atomar auf das Ergebnis. Keine Zusammenarbeit
// innerhalb der Work-Group: bei 2^20 Work-Items konkurrieren 2^20 atomare
// Operationen um dieselbe Speicherzelle.
//
// Neue Konzepte: sycl::atomic_ref

#include "sycl_rahmen.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  sycl::queue q;
  const std::size_t G = rahmen::global_size(cfg.n, 256);

  auto launch = [&](const float* x, float* sum, std::size_t n) {
    q.parallel_for(sycl::range<1>{G}, [=](sycl::id<1> id) {
      float p = 0.0f;
      for (std::size_t k = id[0]; k < n; k += G) p += x[k];   // benachbarte Work-Items
                                                              // lesen benachbarte Werte
      sycl::atomic_ref<float, sycl::memory_order::relaxed, sycl::memory_scope::device,
                       sycl::access::address_space::global_space>
          s(*sum);
      s.fetch_add(p);
    });
  };

  return rahmen::reduktion("reduktion1_atomar", q, cfg, launch);
}
