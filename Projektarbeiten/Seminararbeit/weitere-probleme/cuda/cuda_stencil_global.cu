// CUDA-Gegenstück zu stencil1_2d: ein Thread je innerem Punkt, nur globaler
// Speicher. CUDA verlangt eine Blockgröße (BLOCK x BLOCK, Standard 16).

#include "cuda_rahmen.cuh"

#ifndef BLOCK
#define BLOCK 16
#endif

__global__ void step(const float* in, float* out, std::size_t n) {
  const std::size_t i = blockIdx.y * blockDim.y + threadIdx.y + 1;
  const std::size_t j = blockIdx.x * blockDim.x + threadIdx.x + 1;
  if (i >= n - 1 || j >= n - 1) return;
  out[i * n + j] = rahmen::update(in[(i - 1) * n + j], in[(i + 1) * n + j], in[i * n + j - 1],
                                  in[i * n + j + 1]);
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n, m = n - 2;
  const dim3 block(BLOCK, BLOCK);
  const dim3 grid((m + BLOCK - 1) / BLOCK, (m + BLOCK - 1) / BLOCK);

  auto launch = [&](const float* in, float* out) { step<<<grid, block>>>(in, out, n); };
  return rahmen::stencil("cuda_stencil_global_B" + std::to_string(BLOCK), cfg, launch);
}
