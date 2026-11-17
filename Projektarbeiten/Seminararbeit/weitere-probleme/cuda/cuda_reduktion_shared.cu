// CUDA-Gegenstück zu reduktion2_lokal: Baumreduktion im Shared Memory,
// ein atomicAdd je Block.

#include "cuda_rahmen.cuh"

#ifndef WG
#define WG 256
#endif

__global__ void reduce(const float* x, float* sum, std::size_t n, std::size_t G) {
  __shared__ float s[WG];
  const unsigned lid = threadIdx.x;
  float p = 0.0f;
  for (std::size_t k = blockIdx.x * blockDim.x + lid; k < n; k += G) p += x[k];
  s[lid] = p;
  __syncthreads();
  for (unsigned stride = WG / 2; stride > 0; stride /= 2) {
    if (lid < stride) s[lid] += s[lid + stride];
    __syncthreads();
  }
  if (lid == 0) atomicAdd(sum, s[0]);
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t G = rahmen::global_size(cfg.n, WG);
  auto launch = [&](const float* x, float* sum, std::size_t n) {
    reduce<<<G / WG, WG>>>(x, sum, n, G);
  };
  return rahmen::reduktion("cuda_reduktion_shared_W" + std::to_string(WG), cfg, launch);
}
