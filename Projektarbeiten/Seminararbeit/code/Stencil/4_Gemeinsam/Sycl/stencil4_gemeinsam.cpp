// Stencil Variante 4 – gemeinsamer Speicher (USM shared)
//
// Gegenstück zu Stufe 1 der Matrixmultiplikation: Host und Gerät arbeiten auf
// denselben Zeigern, das Verschieben der Daten übernimmt die Laufzeit. Der
// Kernel ist Zeile für Zeile derselbe wie in Variante 1 (stencil1_2d), die
// einzige Änderung ist die Art der Speicherallokation im Messrahmen.
//
// Erwartung: Bei der Matrixmultiplikation geht der Mehraufwand des gemeinsamen
// Speichers in den vielen Rechenoperationen pro Wert unter. Hier wird jeder
// Wert nur wenige Male gelesen, dafür laufen ITER Zeitschritte über das ganze
// Gitter -- der Unterschied zu eigenem Gerätespeicher (Variante 1) zeigt
// deshalb unverstellt, was das bequemere Speichermodell kostet.

#include "sycl_rahmen.hpp"

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

  return rahmen::stencil_shared("stencil4_gemeinsam", q, cfg, launch);
}
