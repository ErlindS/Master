// Stencil Variante 2 – derselbe Kernel mit linearisiertem 1D-Indexraum
//
// Rechnet exakt dasselbe wie Variante 1, der Indexraum ist aber
// eindimensional; Zeile und Spalte werden aus dem Index berechnet.
// Abram et al. (ISC 2025) berichten, dass DPC++ und AdaptiveCpp 2D-Indexräume
// auf CPUs deutlich schlechter abbilden als 1D – diese Variante prüft das.

#include "sycl_rahmen.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t m = n - 2;   // innere Punkte je Zeile
  sycl::queue q{sycl::property::queue::in_order{}};

  auto launch = [&](const float* in, float* out) {
    q.parallel_for(sycl::range<1>{m * m}, [=](sycl::id<1> idx) {
      const std::size_t i = idx[0] / m + 1, j = idx[0] % m + 1;
      out[i * n + j] = bench::stencil::update(in[(i - 1) * n + j], in[(i + 1) * n + j],
                                              in[i * n + j - 1], in[i * n + j + 1]);
    });
  };

  return rahmen::stencil("stencil2_1d", q, cfg, launch);
}
