// CUDA-Gegenstück zu Stufe 4 (Kachelung im lokalen Speicher)
//
// Begriffe:  SYCL Work-Group  = CUDA Block
//            local_accessor   = __shared__
//            group_barrier    = __syncthreads()
//
// Kachelgröße beim Kompilieren: -DTILE=8|16|32|64. Wie bei SYCL startet
// TILE = 64 auf NVIDIA-GPUs nicht (4096 Threads je Block, erlaubt sind 1024).
// Das Programm meldet dann einen Fehler und endet mit Rückgabewert != 0.

#include "cuda_common.cuh"

#ifndef TILE
#define TILE 16
#endif

__global__ void matmul_tiled(const float* A, const float* B, float* C, std::size_t n) {
  // Lokaler Speicher: eine Kachel von A und eine von B je Block
  __shared__ float tileA[TILE][TILE];
  __shared__ float tileB[TILE][TILE];

  const std::size_t li = threadIdx.y, lj = threadIdx.x;   // Position in der Kachel
  const std::size_t i = blockIdx.y * TILE + li;            // Zeile in C
  const std::size_t j = blockIdx.x * TILE + lj;            // Spalte in C

  float sum = 0.0f;
  for (std::size_t t = 0; t < n; t += TILE) {
    // gemeinsam eine Kachel laden (außerhalb der Matrix: 0)
    tileA[li][lj] = (i < n && t + lj < n) ? A[i * n + t + lj] : 0.0f;
    tileB[li][lj] = (t + li < n && j < n) ? B[(t + li) * n + j] : 0.0f;
    __syncthreads();

    // mit den Daten aus dem schnellen Shared Memory rechnen
    for (std::size_t k = 0; k < TILE; ++k) sum += tileA[li][k] * tileB[k][lj];
    __syncthreads();
  }
  if (i < n && j < n) C[i * n + j] = sum;
}

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);
  const std::string dev = cuda_device_name();

  std::vector<float> hA(n * n), hB(n * n), hC(n * n);
  bench::fill_random(hA.data(), n * n, 1);
  bench::fill_random(hB.data(), n * n, 2);

  float *A, *B, *C;
  CUDA_CHECK(cudaMalloc(&A, bytes));
  CUDA_CHECK(cudaMalloc(&B, bytes));
  CUDA_CHECK(cudaMalloc(&C, bytes));

  const dim3 block(TILE, TILE);
  const dim3 grid((n + TILE - 1) / TILE, (n + TILE - 1) / TILE);

  auto copy_in = [&] {
    CUDA_CHECK(cudaMemcpy(A, hA.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(B, hB.data(), bytes, cudaMemcpyHostToDevice));
  };
  auto kernel = [&] {
    matmul_tiled<<<grid, block>>>(A, B, C, n);
    CUDA_CHECK_LAUNCH();
  };
  auto copy_out = [&] {
    CUDA_CHECK(cudaMemcpy(hC.data(), C, bytes, cudaMemcpyDeviceToHost));
  };

  bench::Timer t0;
  copy_in();
  kernel();
  copy_out();
  const double first = t0.ms();

  std::vector<double> kernel_times, transfer_times;
  for (int r = 0; r < cfg.reps; ++r) {
    bench::Timer tin;
    copy_in();
    const double in_ms = tin.ms();

    bench::Timer tk;
    kernel();
    kernel_times.push_back(tk.ms());

    bench::Timer tout;
    copy_out();
    transfer_times.push_back(in_ms + tout.ms());
  }

  const bool ok = bench::verify(hA.data(), hB.data(), hC.data(), n);
  bench::report("cuda_4_tiling_T" + std::to_string(TILE), dev, n, first,
                bench::median(kernel_times), bench::median(transfer_times), ok);

  CUDA_CHECK(cudaFree(A));
  CUDA_CHECK(cudaFree(B));
  CUDA_CHECK(cudaFree(C));
  return ok ? 0 : 1;
}
