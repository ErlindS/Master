// Stufe 0 – Das "existierende Programm"
//
// Klassische Matrixmultiplikation C = A * B in reinem C++, ein Kern,
// keine Parallelisierung. Dient als Ausgangspunkt und Referenz für
// Laufzeit und Korrektheit.
//
// Bild für die Folie: drei verschachtelte Schleifen. Jedes Element C[i][j]
// ist das Skalarprodukt aus Zeile i von A und Spalte j von B.

#include "common.hpp"

static void matmul(const float* A, const float* B, float* C, std::size_t n) {
  for (std::size_t i = 0; i < n; ++i)          // jede Zeile von C
    for (std::size_t j = 0; j < n; ++j) {      // jede Spalte von C
      float sum = 0.0f;
      for (std::size_t k = 0; k < n; ++k)      // Skalarprodukt
        sum += A[i * n + k] * B[k * n + j];
      C[i * n + j] = sum;
    }
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;

  std::vector<float> A(n * n), B(n * n), C(n * n);
  bench::fill_random(A.data(), n * n, 1);
  bench::fill_random(B.data(), n * n, 2);

  // Erster Lauf (zum Vergleich mit dem "ersten Lauf" der SYCL-Stufen)
  bench::Timer t0;
  matmul(A.data(), B.data(), C.data(), n);
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    bench::Timer t;
    matmul(A.data(), B.data(), C.data(), n);
    times.push_back(t.ms());
  }

  const bool ok = bench::verify(A.data(), B.data(), C.data(), n);
  bench::report("0_sequentiell", "CPU (1 Kern)", n, first, bench::median(times), 0.0, ok);
  return ok ? 0 : 1;
}
