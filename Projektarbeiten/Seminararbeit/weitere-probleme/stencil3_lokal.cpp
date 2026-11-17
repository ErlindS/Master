// Stencil Variante 3 – Kachel mit Halo im lokalen Speicher
//
// Wie Stufe 4 der Matrixmultiplikation: Jede Work-Group (B x B) lädt ihre
// Kachel plus einen Rand von einer Zelle ("Halo") gemeinsam in den lokalen
// Speicher, wartet an einer Barriere und rechnet dann nur noch lokal.
// Jeder Wert wird so einmal statt bis zu fünfmal aus dem globalen Speicher
// gelesen. Ob sich das lohnt, hängt davon ab, wie gut die Caches des Geräts
// das ohnehin abfangen.
//
// Kachelgröße beim Kompilieren: -DBLOCK=8|16|32

#include "sycl_rahmen.hpp"

#ifndef BLOCK
#define BLOCK 16
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  constexpr long B = BLOCK;
  const std::size_t np = (n + B - 1) / B * B;   // Vielfaches der Kachelgröße
  sycl::queue q{sycl::property::queue::in_order{}};

  auto launch = [&](const float* in, float* out) {
    q.submit([&](sycl::handler& h) {
      sycl::local_accessor<float, 2> tile({B + 2, B + 2}, h);
      const long N = static_cast<long>(n);
      h.parallel_for(sycl::nd_range<2>{{np, np}, {B, B}}, [=](sycl::nd_item<2> it) {
        const long li = it.get_local_id(0), lj = it.get_local_id(1);
        const long i0 = long(it.get_group(0)) * B - 1;   // Ecke der Kachel inkl. Halo
        const long j0 = long(it.get_group(1)) * B - 1;

        // (B+2)^2 Werte gemeinsam laden, jedes Work-Item ein bis zwei
        for (long k = li * B + lj; k < (B + 2) * (B + 2); k += B * B) {
          const long ti = k / (B + 2), tj = k % (B + 2);
          const long gi = i0 + ti, gj = j0 + tj;
          tile[ti][tj] = (gi >= 0 && gi < N && gj >= 0 && gj < N) ? in[gi * N + gj] : 0.0f;
        }
        sycl::group_barrier(it.get_group());

        const long i = i0 + 1 + li, j = j0 + 1 + lj;
        if (i >= 1 && i < N - 1 && j >= 1 && j < N - 1)
          out[i * N + j] = bench::stencil::update(tile[li][lj + 1], tile[li + 2][lj + 1],
                                                  tile[li + 1][lj], tile[li + 1][lj + 2]);
      });
    });
  };

  return rahmen::stencil("stencil3_lokal_B" + std::to_string(B), q, cfg, launch);
}
