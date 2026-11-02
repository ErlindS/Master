// cuda_triad.cu – native Referenz für die Triade auf der NVIDIA-GPU
//
//   ./cuda_triad triad <n> [wiederholungen]

#include <cuda_runtime.h>

#include "common.hpp"

#define CK(x)                                                                         \
  do {                                                                                \
    cudaError_t e_ = (x);                                                             \
    if (e_ != cudaSuccess) {                                                          \
      std::fprintf(stderr, "CUDA-Fehler: %s (%s:%d)\n", cudaGetErrorString(e_),      \
                   __FILE__, __LINE__);                                               \
      std::exit(1);                                                                   \
    }                                                                                 \
  } while (0)

__global__ void triad(float* a, const float* b, const float* c, float s, size_t n) {
  const size_t i = size_t(blockIdx.x) * blockDim.x + threadIdx.x;
  if (i < n) a[i] = b[i] + s * c[i];
}

int main(int argc, char** argv) {
  const auto args = va::parse_args(argc, argv, "triad");
  const std::size_t n = args.n;
  const std::size_t bytes = n * sizeof(float);
  const float s = va::kTriadScalar;

  cudaDeviceProp prop{};
  CK(cudaGetDeviceProperties(&prop, 0));
  const va::Kontext k{"cuda", "gpu", prop.name, "triad", "triad", n};

  std::vector<float> ha(n), hb(n), hc(n);
  va::fill_random(hb.data(), n, 3);
  va::fill_random(hc.data(), n, 4);

  float *a, *b, *c;
  CK(cudaMalloc(&a, bytes));
  CK(cudaMalloc(&b, bytes));
  CK(cudaMalloc(&c, bytes));
  CK(cudaMemcpy(b, hb.data(), bytes, cudaMemcpyHostToDevice));
  CK(cudaMemcpy(c, hc.data(), bytes, cudaMemcpyHostToDevice));

  const unsigned block = 256;
  const unsigned grid = unsigned((n + block - 1) / block);

  va::Timer t0;
  triad<<<grid, block>>>(a, b, c, s, n);
  CK(cudaDeviceSynchronize());
  const double first = t0.ms();
  va::zeile(k, 0, first, va::triad_gbs(n, first), "GB/s", "erster_lauf");

  cudaEvent_t start, stop;
  CK(cudaEventCreate(&start));
  CK(cudaEventCreate(&stop));
  for (int r = 1; r <= args.reps; ++r) {
    CK(cudaEventRecord(start));
    triad<<<grid, block>>>(a, b, c, s, n);
    CK(cudaEventRecord(stop));
    CK(cudaEventSynchronize(stop));
    float ms = 0.0f;
    CK(cudaEventElapsedTime(&ms, start, stop));
    va::zeile(k, r, ms, va::triad_gbs(n, ms), "GB/s", "ok");
  }

  CK(cudaMemcpy(ha.data(), a, bytes, cudaMemcpyDeviceToHost));
  const bool ok = va::verify_triad(ha.data(), hb.data(), hc.data(), s, n);
  if (!ok) va::zeile(k, 0, NAN, 0.0, "GB/s", "falsch");

  cudaFree(a);
  cudaFree(b);
  cudaFree(c);
  return ok ? 0 : 1;
}
