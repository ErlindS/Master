// Reduktion Variante 3 – Gruppenkollektiv reduce_over_group
//
// Dieselbe Struktur wie Variante 2, aber die Baumreduktion übernimmt die
// SYCL-2020-Funktion reduce_over_group. Die Implementierung kann dafür
// Hardware-Befehle nutzen (z. B. Warp-Shuffles auf NVIDIA), ohne dass das
// Programm sie kennt.
//
// Neue Konzepte: Gruppenkollektive (reduce_over_group)

#include "sycl_rahmen.hpp"

#ifndef WG
#define WG 256
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  sycl::queue q;
  constexpr std::size_t W = WG;
  const std::size_t G = rahmen::global_size(cfg.n, W);

  auto launch = [&](const float* x, float* sum, std::size_t n) {
    q.parallel_for(sycl::nd_range<1>{G, W}, [=](sycl::nd_item<1> it) {
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
  };

  return rahmen::reduktion("reduktion3_gruppe_W" + std::to_string(W), q, cfg, launch);
}
