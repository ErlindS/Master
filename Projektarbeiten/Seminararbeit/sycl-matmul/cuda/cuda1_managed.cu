// CUDA-Gegenstück zu Stufe 1 (USM shared)
//
// cudaMallocManaged entspricht sycl::malloc_shared: derselbe Zeiger gilt auf
// Host und GPU, der Treiber verschiebt die Daten bei Bedarf.
//
// Unterschied zu SYCL: CUDA verlangt immer eine Blockgröße. Einen einfachen
// Indexraum ohne Work-Group-Größe (sycl::range) gibt es nicht. Verwendet
// wird BLOCK x BLOCK (Standard 16, -DBLOCK=...).

#include "cuda_common.cuh"

#ifndef BLOCK
#define BLOCK 16
#endif

// Ein Thread je Element von C. x = Spalte j (schnellste Dimension, wie
// Dimension 1 der SYCL-range), y = Zeile i.
__global__ void matmul(const float* A, const float* B, float* C, std::size_t n) {
  const std::size_t i = blockIdx.y * blockDim.y + threadIdx.y;
  const std::size_t j = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= n || j >= n) return;
  float sum = 0.0f;
  for (std::size_t k = 0; k < n; ++k) sum += A[i * n + k] * B[k * n + j];
  C[i * n + j] = sum;
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::string dev = cuda_device_name();

  float *A, *B, *C;
  CUDA_CHECK(cudaMallocManaged(&A, n * n * sizeof(float)));
  CUDA_CHECK(cudaMallocManaged(&B, n * n * sizeof(float)));
  CUDA_CHECK(cudaMallocManaged(&C, n * n * sizeof(float)));
  bench::fill_random(A, n * n, 1);
  bench::fill_random(B, n * n, 2);

  const dim3 block(BLOCK, BLOCK);
  const dim3 grid((n + BLOCK - 1) / BLOCK, (n + BLOCK - 1) / BLOCK);
  auto run = [&] {
    matmul<<<grid, block>>>(A, B, C, n);
    CUDA_CHECK_LAUNCH();   // entspricht .wait()
  };

  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    bench::Timer t;
    run();
    times.push_back(t.ms());
  }

  const bool ok = bench::verify(A, B, C, n);
  bench::report("cuda_1_managed", dev, n, first, bench::median(times), 0.0, ok);

  CUDA_CHECK(cudaFree(A));
  CUDA_CHECK(cudaFree(B));
  CUDA_CHECK(cudaFree(C));
  return ok ? 0 : 1;
}
