// Reduktion Variante 2 – Baumreduktion im lokalen Speicher
//
// Wie Variante 1, aber die Work-Group fasst ihre Teilsummen zuerst selbst
// zusammen: Jede Runde halbiert die Zahl der aktiven Work-Items, eine
// Barriere trennt die Runden. Danach nur noch ein Atomic je Work-Group.
//
// Neue Konzepte: lokaler Speicher und Barriere für Kooperation (statt nur
// für Wiederverwendung wie bei der Matrixmultiplikation)

#include "sycl_rahmen.hpp"

#ifndef WG
#define WG 256   // Zweierpotenz
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  sycl::queue q;
  constexpr std::size_t W = WG;
  const std::size_t G = rahmen::global_size(cfg.n, W);

  auto launch = [&](const float* x, float* sum, std::size_t n) {
    q.submit([&](sycl::handler& h) {
      sycl::local_accessor<float, 1> s(W, h);
      h.parallel_for(sycl::nd_range<1>{G, W}, [=](sycl::nd_item<1> it) {
        const std::size_t lid = it.get_local_id(0);
        float p = 0.0f;
        for (std::size_t k = it.get_global_id(0); k < n; k += G) p += x[k];
        s[lid] = p;
        sycl::group_barrier(it.get_group());

        for (std::size_t stride = W / 2; stride > 0; stride /= 2) {
          if (lid < stride) s[lid] += s[lid + stride];
          sycl::group_barrier(it.get_group());
        }
        if (lid == 0) {
          sycl::atomic_ref<float, sycl::memory_order::relaxed, sycl::memory_scope::device,
                           sycl::access::address_space::global_space>
              r(*sum);
          r.fetch_add(s[0]);
        }
      });
    });
  };

  return rahmen::reduktion("reduktion2_lokal_W" + std::to_string(W), q, cfg, launch);
}
