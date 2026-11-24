// CUDA-Gegenstück zu reduktion1_atomar: private Teilsumme, dann ein
// atomicAdd je Thread. atomicAdd(float) ist auf NVIDIA ein Hardwarebefehl.

#include "cuda_rahmen.cuh"

__global__ void reduce(const float* x, float* sum, std::size_t n, std::size_t G) {
  float p = 0.0f;
  for (std::size_t k = blockIdx.x * blockDim.x + threadIdx.x; k < n; k += G) p += x[k];
  atomicAdd(sum, p);
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t W = 256, G = rahmen::global_size(cfg.n, W);
  auto launch = [&](const float* x, float* sum, std::size_t n) {
    reduce<<<G / W, W>>>(x, sum, n, G);
  };
  return rahmen::reduktion("cuda_reduktion_atomar", cfg, launch);
}
