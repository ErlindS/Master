// cpu_triad.cpp – native Referenz für die Triade auf der CPU (OpenMP)
//
//   ./cpu_triad triad <n> [wiederholungen]
//
// Threads über OMP_NUM_THREADS. Die Vektoren werden parallel initialisiert,
// damit die Speicherseiten bei dem Kern liegen, der sie später bearbeitet
// (First Touch).

#include <omp.h>

#include <memory>

#include "common.hpp"

int main(int argc, char** argv) {
  const auto args = va::parse_args(argc, argv, "triad");
  const std::size_t n = args.n;
  const float s = va::kTriadScalar;
  const va::Kontext k{"openmp", "cpu", va::cpu_name(), "triad", "triad", n};

  std::unique_ptr<float[]> a(new float[n]), b(new float[n]), c(new float[n]);
  float* pa = a.get();
  float* pb = b.get();
  float* pc = c.get();

#pragma omp parallel for schedule(static)
  for (std::size_t i = 0; i < n; ++i) {
    pa[i] = 0.0f;
    pb[i] = float(i % 1000) * 1e-3f;
    pc[i] = float((i * 7) % 1000) * 1e-3f;
  }

  auto run = [&] {
#pragma omp parallel for schedule(static)
    for (std::size_t i = 0; i < n; ++i) pa[i] = pb[i] + s * pc[i];
  };

  va::Timer t0;
  run();
  const double first = t0.ms();
  va::zeile(k, 0, first, va::triad_gbs(n, first), "GB/s", "erster_lauf");

  for (int r = 1; r <= args.reps; ++r) {
    va::Timer t;
    run();
    const double ms = t.ms();
    va::zeile(k, r, ms, va::triad_gbs(n, ms), "GB/s", "ok");
  }

  const bool ok = va::verify_triad(pa, pb, pc, s, n);
  if (!ok) va::zeile(k, 0, NAN, 0.0, "GB/s", "falsch");
  return ok ? 0 : 1;
}
