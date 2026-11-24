// CUDA-Gegenstück zu Stufe 2 (USM device) und Stufe 3 (nd_range)
//
// cudaMalloc + cudaMemcpy entsprechen sycl::malloc_device + q.memcpy.
// Da CUDA die Blockgröße immer verlangt, fallen Stufe 2 und 3 hier
// zusammen: Mit BLOCK = 16 entspricht das Programm Stufe 3 mit WGSIZE = 16.
//
// Messung wie in Stufe 2/3: Kernel- und Transferzeit getrennt.

#include "cuda_common.cuh"

#ifndef BLOCK
#define BLOCK 16
#endif

__global__ void matmul(const float* A, const float* B, float* C, std::size_t n) {
  const std::size_t i = blockIdx.y * blockDim.y + threadIdx.y;   // Zeile
  const std::size_t j = blockIdx.x * blockDim.x + threadIdx.x;   // Spalte
  if (i >= n || j >= n) return;
  float sum = 0.0f;
  for (std::size_t k = 0; k < n; ++k) sum += A[i * n + k] * B[k * n + j];
  C[i * n + j] = sum;
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

  const dim3 block(BLOCK, BLOCK);
  const dim3 grid((n + BLOCK - 1) / BLOCK, (n + BLOCK - 1) / BLOCK);

  auto copy_in = [&] {
    CUDA_CHECK(cudaMemcpy(A, hA.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(B, hB.data(), bytes, cudaMemcpyHostToDevice));
  };
  auto kernel = [&] {
    matmul<<<grid, block>>>(A, B, C, n);
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
  bench::report("cuda_2_device_B" + std::to_string(BLOCK), dev, n, first,
                bench::median(kernel_times), bench::median(transfer_times), ok);

  CUDA_CHECK(cudaFree(A));
  CUDA_CHECK(cudaFree(B));
  CUDA_CHECK(cudaFree(C));
  return ok ? 0 : 1;
}
