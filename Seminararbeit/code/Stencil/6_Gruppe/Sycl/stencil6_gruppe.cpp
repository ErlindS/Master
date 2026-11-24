// Stencil Variante 6 – selbst gewählte Work-Group-Größe, kein lokaler Speicher
//
// Gegenstück zu Stufe 3 der Matrixmultiplikation. Variante 1 übergibt einen
// einfachen Indexraum (sycl::range) und lässt die Laufzeit die Work-Group-Größe
// bestimmen; Variante 3 legt sie fest UND nutzt lokalen Speicher. Diese
// Variante dazwischen legt nur die Gruppengröße fest. Erst damit lässt sich
// trennen, was die Gruppeneinteilung bringt und was der lokale Speicher.
//
// Die Gruppe ist W x W Work-Items, der Indexraum wird auf ein Vielfaches von W
// aufgerundet; überzählige Work-Items tun nichts (Randabfrage im Kernel).
//
// Gruppengröße wählen:
//   ./stencil6_gruppe <n> [wiederholungen] [W]      (Vorgabe W = 16)
// Dieselbe W-Reihe wie bei der Matrixmultiplikation macht die
// Performance-Portabilität der Gruppengröße auch hier messbar.

#include "sycl_rahmen.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t W = (argc > 3) ? std::strtoul(argv[3], nullptr, 10) : 16;
  if (W == 0 || W > n) {
    std::fprintf(stderr, "Gruppengröße W muss zwischen 1 und n liegen\n");
    return 1;
  }
  const std::size_t np = (n + W - 1) / W * W;   // Vielfaches der Gruppengröße
  sycl::queue q{sycl::property::queue::in_order{}};

  auto launch = [&](const float* in, float* out) {
    q.parallel_for(sycl::nd_range<2>{{np, np}, {W, W}}, [=](sycl::nd_item<2> it) {
      const std::size_t i = it.get_global_id(0), j = it.get_global_id(1);
      if (i >= 1 && i + 1 < n && j >= 1 && j + 1 < n)
        out[i * n + j] = bench::stencil::update(in[(i - 1) * n + j], in[(i + 1) * n + j],
                                                in[i * n + j - 1], in[i * n + j + 1]);
    });
  };

  return rahmen::stencil("stencil6_gruppe_W" + std::to_string(W), q, cfg, launch);
}
