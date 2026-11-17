// cuda_rahmen.cuh – Messrahmen der CUDA-Gegenstücke (wie ../sycl_rahmen.hpp)
#pragma once

#include <cuda_runtime.h>
#include "../probleme.hpp"

#define CUDA_CHECK(call)                                                        \
  do {                                                                          \
    const cudaError_t err_ = (call);                                            \
    if (err_ != cudaSuccess) {                                                  \
      std::fprintf(stderr, "CUDA-Fehler %s in %s:%d: %s\n", #call, __FILE__,    \
                   __LINE__, cudaGetErrorString(err_));                         \
      std::exit(2);                                                             \
    }                                                                           \
  } while (0)

namespace rahmen {

inline std::string device_name() {
  int dev = 0;
  CUDA_CHECK(cudaGetDevice(&dev));
  cudaDeviceProp p{};
  CUDA_CHECK(cudaGetDeviceProperties(&p, dev));
  return p.name;
}

// Gleiche Formel wie die Host-Referenz (bench::stencil::update)
__device__ __forceinline__ float update(float up, float down, float left, float right) {
  return 0.25f * (((up + down) + left) + right);
}

// Stencil: launch(in, out) startet einen Zeitschritt. Alle Kernel laufen im
// Standard-Stream und damit nacheinander (wie die in-order-Queue in SYCL).
template <class Launch>
int stencil(const std::string& name, const bench::Config& cfg, Launch launch) {
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);
  const std::vector<float> start = bench::stencil::init(n);
  const std::vector<float> ref = bench::stencil::reference(n, ITER);

  float *a, *b;
  CUDA_CHECK(cudaMalloc(&a, bytes));
  CUDA_CHECK(cudaMalloc(&b, bytes));

  auto reset = [&] {
    CUDA_CHECK(cudaMemcpy(a, start.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(b, start.data(), bytes, cudaMemcpyHostToDevice));
  };
  float* result = nullptr;
  auto run = [&] {
    float* in = a;
    float* out = b;
    for (int it = 0; it < ITER; ++it) {
      launch(in, out);
      std::swap(in, out);
    }
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    result = in;
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
  CUDA_CHECK(cudaMemcpy(got.data(), result, bytes, cudaMemcpyDeviceToHost));
  const bool ok = bench::stencil::verify(got.data(), ref, n);
  bench::report_bw(name, device_name(), n, first, bench::median(times),
                   bench::stencil::bytes(n, ITER), ok);

  CUDA_CHECK(cudaFree(a));
  CUDA_CHECK(cudaFree(b));
  return ok ? 0 : 1;
}

// Reduktion: launch(x, sum, n) startet die Reduktion, *sum ist vorher 0.
template <class Launch>
int reduktion(const std::string& name, const bench::Config& cfg, Launch launch) {
  const std::size_t n = cfg.n;
  const std::vector<float> hx = bench::reduktion::init(n);
  const double ref = bench::reduktion::reference(hx);

  float *x, *sum;
  CUDA_CHECK(cudaMalloc(&x, n * sizeof(float)));
  CUDA_CHECK(cudaMalloc(&sum, sizeof(float)));
  CUDA_CHECK(cudaMemcpy(x, hx.data(), n * sizeof(float), cudaMemcpyHostToDevice));

  auto once = [&] {
    launch(x, sum, n);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
  };

  CUDA_CHECK(cudaMemset(sum, 0, sizeof(float)));
  bench::Timer t0;
  once();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    CUDA_CHECK(cudaMemset(sum, 0, sizeof(float)));
    CUDA_CHECK(cudaDeviceSynchronize());
    bench::Timer t;
    once();
    times.push_back(t.ms());
  }

  float got = 0.0f;
  CUDA_CHECK(cudaMemcpy(&got, sum, sizeof(float), cudaMemcpyDeviceToHost));
  const bool ok = bench::reduktion::verify(got, ref);
  bench::report_bw(name, device_name(), n, first, bench::median(times),
                   bench::reduktion::bytes(n), ok);

  CUDA_CHECK(cudaFree(x));
  CUDA_CHECK(cudaFree(sum));
  return ok ? 0 : 1;
}

inline std::size_t global_size(std::size_t n, std::size_t wg) {
  const std::size_t g = std::min<std::size_t>(n, std::size_t(1) << 20);
  return (g + wg - 1) / wg * wg;
}

}  // namespace rahmen
