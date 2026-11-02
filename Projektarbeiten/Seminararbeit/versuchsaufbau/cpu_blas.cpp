// cpu_blas.cpp – OpenBLAS-sgemm als Baseline auf der CPU
//
//   ./cpu_blas sgemm <n> [wiederholungen]
//
// Herstellerunabhängige, hochoptimierte Bibliothek als Obergrenze für die
// CPU-Effizienz. Threads über OPENBLAS_NUM_THREADS (Standard: alle Kerne).

#include <cblas.h>

#include "common.hpp"

int main(int argc, char** argv) {
  const auto args = va::parse_args(argc, argv, "sgemm");
  const std::size_t n = args.n;
  const va::Kontext k{"openblas", "cpu", va::cpu_name(), "gemm", "sgemm", n};

  std::vector<float> A(n * n), B(n * n), C(n * n);
  va::fill_random(A.data(), n * n, 1);
  va::fill_random(B.data(), n * n, 2);

  auto run = [&] {
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, int(n), int(n), int(n), 1.0f,
                A.data(), int(n), B.data(), int(n), 0.0f, C.data(), int(n));
  };

  va::Timer t0;
  run();
  const double first = t0.ms();
  va::zeile(k, 0, first, va::gemm_gflops(n, first), "GFLOPS", "erster_lauf");

  for (int r = 1; r <= args.reps; ++r) {
    va::Timer t;
    run();
    const double ms = t.ms();
    va::zeile(k, r, ms, va::gemm_gflops(n, ms), "GFLOPS", "ok");
  }

  const bool ok = va::verify_gemm(A.data(), B.data(), C.data(), n);
  if (!ok) va::zeile(k, 0, NAN, 0.0, "GFLOPS", "falsch");
  return ok ? 0 : 1;
}
