// Reduktion Variante 4 – eingebaute Reduktion (sycl::reduction)
//
// Höchste Abstraktion: Das Programm gibt nur an, WAS reduziert wird (Summe
// in *sum), und jedes Work-Item liefert einen Wert. Wie die Teilsummen
// zusammengeführt werden und wie viele Work-Items es gibt, entscheidet die
// Implementierung. Gegenstück zu Buffern bei der Matrixmultiplikation:
// bequem, aber ohne Kontrolle.
//
// Neue Konzepte: sycl::reduction, Reducer

#include "sycl_rahmen.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  sycl::queue q;

  auto launch = [&](const float* x, float* sum, std::size_t n) {
    q.parallel_for(sycl::range<1>{n}, sycl::reduction(sum, sycl::plus<float>()),
                   [=](sycl::id<1> i, auto& s) { s += x[i]; });
  };

  return rahmen::reduktion("reduktion4_builtin", q, cfg, launch);
}
