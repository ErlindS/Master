// Obergrenze: cuBLAS SGEMM (herstelleroptimierte Bibliothek)
//
// Kein Gegenstück zu einer Stufe, sondern Referenz dafür, wie weit die
// handgeschriebenen Kernel (CUDA wie SYCL) vom Möglichen entfernt sind.
//
// cuBLAS rechnet spaltenweise (column-major). Für zeilenweise gespeicherte
// Matrizen gilt C^T = B^T * A^T, also wird sgemm mit vertauschten
// Argumenten B, A aufgerufen.

#include <cublas_v2.h>
#include "cuda_common.cuh"

#define CUBLAS_CHECK(call)                                                      \
  do {                                                                          \
    const cublasStatus_t st_ = (call);                                          \
    if (st_ != CUBLAS_STATUS_SUCCESS) {                                         \
      std::fprintf(stderr, "cuBLAS-Fehler %d in %s:%d\n", int(st_), __FILE__,   \
                   __LINE__);                                                   \
      std::exit(2);                                                             \
    }                                                                           \
  } while (0)

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

  cublasHandle_t h;
  CUBLAS_CHECK(cublasCreate(&h));
  // Tensor-Cores mit reduzierter Genauigkeit (TF32) ausschließen, damit
  // der Vergleich mit den FP32-Kerneln fair bleibt.
  CUBLAS_CHECK(cublasSetMathMode(h, CUBLAS_PEDANTIC_MATH));

  const float alpha = 1.0f, beta = 0.0f;
  const int ni = static_cast<int>(n);

  auto copy_in = [&] {
    CUDA_CHECK(cudaMemcpy(A, hA.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(B, hB.data(), bytes, cudaMemcpyHostToDevice));
  };
  auto kernel = [&] {
    CUBLAS_CHECK(cublasSgemm(h, CUBLAS_OP_N, CUBLAS_OP_N, ni, ni, ni, &alpha,
                             B, ni, A, ni, &beta, C, ni));
    CUDA_CHECK(cudaDeviceSynchronize());
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
  bench::report("cuda_cublas", dev, n, first, bench::median(kernel_times),
                bench::median(transfer_times), ok);

  CUBLAS_CHECK(cublasDestroy(h));
  CUDA_CHECK(cudaFree(A));
  CUDA_CHECK(cudaFree(B));
  CUDA_CHECK(cudaFree(C));
  return ok ? 0 : 1;
}
