// sycl_rahmen.hpp – gemeinsamer Messrahmen der SYCL-Varianten
//
// Jedes Programm liefert nur seinen Kernel (als Funktion "launch"), der Rest
// – Speicher, Kopieren, Zeitmessung, Prüfung, Ausgabe – steht hier, damit sich
// die Varianten nur im Kernel unterscheiden.
#pragma once

#include <sycl/sycl.hpp>
#include "probleme.hpp"

namespace rahmen {

inline std::string device_name(const sycl::queue& q) {
  return q.get_device().get_info<sycl::info::device::name>();
}

// Stencil: launch(in, out) reicht EINEN Zeitschritt ein. Die Queue muss
// in-order sein, damit aufeinanderfolgende Zeitschritte nacheinander laufen,
// ohne dass der Host dazwischen wartet. Mit sync_each wartet der Host nach
// jedem Schritt (macht den Overhead pro Kernelstart sichtbar).
template <class Launch>
int stencil(const std::string& name, sycl::queue& q, const bench::Config& cfg, Launch launch,
            bool sync_each = false) {
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);
  const std::vector<float> start = bench::stencil::init(n);
  const std::vector<float> ref = bench::stencil::reference(n, ITER);

  float* a = sycl::malloc_device<float>(n * n, q);
  float* b = sycl::malloc_device<float>(n * n, q);

  auto reset = [&] {   // beide Puffer enthalten den festen Rand
    q.memcpy(a, start.data(), bytes);
    q.memcpy(b, start.data(), bytes);
    q.wait();
  };
  float* result = nullptr;
  auto run = [&] {
    float* in = a;
    float* out = b;
    for (int it = 0; it < ITER; ++it) {
      launch(in, out);
      if (sync_each) q.wait();
      std::swap(in, out);
    }
    q.wait();
    result = in;   // nach dem Tausch liegt das Ergebnis in "in"
  };

  reset();
  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    reset();
    bench::Timer t;
    run();
    times.push_back(t.ms());
  }

  std::vector<float> got(n * n);
  q.memcpy(got.data(), result, bytes).wait();
  const bool ok = bench::stencil::verify(got.data(), ref, n);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::stencil::bytes(n, ITER), ok);

  sycl::free(a, q);
  sycl::free(b, q);
  return ok ? 0 : 1;
}

// Reduktion: launch(x, sum, n) reicht die Reduktion ein, *sum ist vorher 0.
template <class Launch>
int reduktion(const std::string& name, sycl::queue& q, const bench::Config& cfg, Launch launch) {
  const std::size_t n = cfg.n;
  const std::vector<float> hx = bench::reduktion::init(n);
  const double ref = bench::reduktion::reference(hx);

  float* x = sycl::malloc_device<float>(n, q);
  float* sum = sycl::malloc_device<float>(1, q);
  q.memcpy(x, hx.data(), n * sizeof(float)).wait();   // Daten liegen vorab auf dem Gerät

  auto run = [&] {
    q.memset(sum, 0, sizeof(float)).wait();
    launch(x, sum, n);
    q.wait();
  };

  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    q.memset(sum, 0, sizeof(float)).wait();   // nicht mitgemessen
    bench::Timer t;
    launch(x, sum, n);
    q.wait();
    times.push_back(t.ms());
  }

  float got = 0.0f;
  q.memcpy(&got, sum, sizeof(float)).wait();
  const bool ok = bench::reduktion::verify(got, ref);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::reduktion::bytes(n), ok);

  sycl::free(x, q);
  sycl::free(sum, q);
  return ok ? 0 : 1;
}

// Anzahl Work-Items für die Reduktionen mit eigener Schleife: höchstens
// 2^20, jedes summiert zuerst n / global Werte privat auf.
inline std::size_t global_size(std::size_t n, std::size_t wg) {
  const std::size_t g = std::min<std::size_t>(n, std::size_t(1) << 20);
  return (g + wg - 1) / wg * wg;
}

}  // namespace rahmen
