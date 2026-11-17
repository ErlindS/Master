// CUDA-Gegenstück zu stencil3_lokal: Kachel mit Halo im Shared Memory.

#include "cuda_rahmen.cuh"

#ifndef BLOCK
#define BLOCK 16
#endif

__global__ void step(const float* in, float* out, long N) {
  constexpr long B = BLOCK;
  __shared__ float tile[B + 2][B + 2];
  const long li = threadIdx.y, lj = threadIdx.x;
  const long i0 = long(blockIdx.y) * B - 1, j0 = long(blockIdx.x) * B - 1;

  for (long k = li * B + lj; k < (B + 2) * (B + 2); k += B * B) {
    const long ti = k / (B + 2), tj = k % (B + 2);
    const long gi = i0 + ti, gj = j0 + tj;
    tile[ti][tj] = (gi >= 0 && gi < N && gj >= 0 && gj < N) ? in[gi * N + gj] : 0.0f;
  }
  __syncthreads();

  const long i = i0 + 1 + li, j = j0 + 1 + lj;
  if (i >= 1 && i < N - 1 && j >= 1 && j < N - 1)
    out[i * N + j] = rahmen::update(tile[li][lj + 1], tile[li + 2][lj + 1], tile[li + 1][lj],
                                    tile[li + 1][lj + 2]);
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const dim3 block(BLOCK, BLOCK);
  const dim3 grid((n + BLOCK - 1) / BLOCK, (n + BLOCK - 1) / BLOCK);

  auto launch = [&](const float* in, float* out) { step<<<grid, block>>>(in, out, long(n)); };
  return rahmen::stencil("cuda_stencil_shared_B" + std::to_string(BLOCK), cfg, launch);
}
