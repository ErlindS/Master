// Stencil Variante 1 – einfacher 2D-Indexraum, nur globaler Speicher
//
// Entspricht Stufe 1/2 der Matrixmultiplikation: ein Work-Item je innerem
// Gitterpunkt, die Laufzeit wählt die Work-Group-Größe.
//
// Neu gegenüber der Matrixmultiplikation: in-order-Queue. Die ITER
// Zeitschritte werden hintereinander eingereicht, ohne dass der Host
// zwischendurch wartet; die Queue hält die Reihenfolge ein.
//
// Mit -DSYNC_JEDE wartet der Host nach jedem Zeitschritt (Variante
// stencil1_2d_sync), um den Overhead pro Kernelstart zu messen.

#include "sycl_rahmen.hpp"

#ifdef SYNC_JEDE
constexpr bool sync_each = true;
#else
constexpr bool sync_each = false;
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  sycl::queue q{sycl::property::queue::in_order{}};

  auto launch = [&](const float* in, float* out) {
    q.parallel_for(sycl::range<2>{n - 2, n - 2}, [=](sycl::id<2> idx) {
      const std::size_t i = idx[0] + 1, j = idx[1] + 1;   // nur innere Punkte
      out[i * n + j] = bench::stencil::update(in[(i - 1) * n + j], in[(i + 1) * n + j],
                                              in[i * n + j - 1], in[i * n + j + 1]);
    });
  };

  const std::string name = sync_each ? "stencil1_2d_sync" : "stencil1_2d";
  return rahmen::stencil(name, q, cfg, launch, sync_each);
}
