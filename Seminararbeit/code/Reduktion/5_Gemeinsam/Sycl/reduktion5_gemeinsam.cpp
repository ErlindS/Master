// Reduktion Variante 5 – gemeinsamer Speicher (USM shared)
//
// Gegenstück zu Stufe 1 der Matrixmultiplikation: Eingabevektor und Summe
// liegen im gemeinsamen Speicher, der Host schreibt die Werte direkt hinein,
// das Verschieben übernimmt die Laufzeit. Der Kernel ist derselbe wie in
// Variante 1 (reduktion1_atomar); nur das Speichermodell unterscheidet sich.
//
// Besonders aussagekräftig, weil die Reduktion jeden Wert genau einmal liest:
// Hier kann sich kein Rechenaufwand über die Kosten des Speichermodells legen.

#include "sycl_rahmen.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  sycl::queue q;
  const std::size_t G = rahmen::global_size(cfg.n, 256);

  auto launch = [&](const float* x, float* sum, std::size_t n) {
    q.parallel_for(sycl::range<1>{G}, [=](sycl::id<1> id) {
      float p = 0.0f;
      for (std::size_t k = id[0]; k < n; k += G) p += x[k];
      sycl::atomic_ref<float, sycl::memory_order::relaxed, sycl::memory_scope::device,
                       sycl::access::address_space::global_space>
          s(*sum);
      s.fetch_add(p);
    });
  };

  return rahmen::reduktion_shared("reduktion5_gemeinsam", q, cfg, launch);
}
