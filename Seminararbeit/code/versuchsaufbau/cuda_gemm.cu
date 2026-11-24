// cuda_gemm.cu – native CUDA-Vergleichsvarianten auf der NVIDIA-GPU
//
//   ./cuda_gemm <variante> <n> [wiederholungen]
//
// Varianten:
//   naiv            derselbe Algorithmus wie SYCL "naiv"
//   tile8 … tile64  derselbe Kachel-Algorithmus wie SYCL "tileT"
//   cublas          cublasSgemm als herstelleroptimierte Obergrenze
//
// Der Vergleich SYCL tileT gegen CUDA tileT beantwortet, was die
// Portabilität bei identischem Algorithmus kostet; cuBLAS dient als Baseline
// für die Effizienz. Zeitmessung über CUDA-Events (nur Kernel).

#include <cublas_v2.h>
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

__global__ void gemm_naiv(const float* A, const float* B, float* C, int n) {
  const int i = blockIdx.y * blockDim.y + threadIdx.y;
  const int j = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= n || j >= n) return;
  float sum = 0.0f;
  for (int k = 0; k < n; ++k) sum += A[size_t(i) * n + k] * B[size_t(k) * n + j];
  C[size_t(i) * n + j] = sum;
}

// KERNEL-BEGIN tiled
template <int T>
__global__ void gemm_tile(const float* A, const float* B, float* C, int n) {
  __shared__ float tA[T][T];
  __shared__ float tB[T][T];
  const int li = threadIdx.y, lj = threadIdx.x;
  const int i = blockIdx.y * T + li, j = blockIdx.x * T + lj;
  float sum = 0.0f;
  for (int t = 0; t < n; t += T) {
    tA[li][lj] = (i < n && t + lj < n) ? A[size_t(i) * n + t + lj] : 0.0f;
    tB[li][lj] = (t + li < n && j < n) ? B[size_t(t + li) * n + j] : 0.0f;
    __syncthreads();
    for (int k = 0; k < T; ++k) sum += tA[li][k] * tB[k][lj];
    __syncthreads();
  }
  if (i < n && j < n) C[size_t(i) * n + j] = sum;
}
// KERNEL-END tiled

namespace {

template <int T>
void launch_tile(const float* A, const float* B, float* C, int n) {
  const dim3 block(T, T), grid((n + T - 1) / T, (n + T - 1) / T);
  gemm_tile<T><<<grid, block>>>(A, B, C, n);
}

void launch_naiv(const float* A, const float* B, float* C, int n) {
  const dim3 block(16, 16), grid((n + 15) / 16, (n + 15) / 16);
  gemm_naiv<<<grid, block>>>(A, B, C, n);
}

cublasHandle_t g_handle;

// cuBLAS rechnet spaltenweise: C^T = B^T · A^T entspricht C = A · B zeilenweise.
void launch_cublas(const float* A, const float* B, float* C, int n) {
  const float one = 1.0f, zero = 0.0f;
  cublasSgemm(g_handle, CUBLAS_OP_N, CUBLAS_OP_N, n, n, n, &one, B, n, A, n, &zero, C, n);
}

}  // namespace

int main(int argc, char** argv) {
  const auto args = va::parse_args(argc, argv, "naiv|tile8|tile16|tile32|tile64|cublas");
  const int n = int(args.n);
  const std::size_t bytes = args.n * args.n * sizeof(float);

  using Launch = void (*)(const float*, const float*, float*, int);
  Launch fn = nullptr;
  if (args.variante == "naiv") fn = launch_naiv;
  if (args.variante == "tile8") fn = launch_tile<8>;
  if (args.variante == "tile16") fn = launch_tile<16>;
  if (args.variante == "tile32") fn = launch_tile<32>;
  if (args.variante == "tile64") fn = launch_tile<64>;
  if (args.variante == "cublas") fn = launch_cublas;
  if (!fn) {
    std::fprintf(stderr, "unbekannte Variante: %s\n", args.variante.c_str());
    return 2;
  }

  cudaDeviceProp prop{};
  CK(cudaGetDeviceProperties(&prop, 0));
  const va::Kontext k{"cuda", "gpu", prop.name, "gemm", args.variante, args.n};

  std::vector<float> hA(args.n * args.n), hB(args.n * args.n), hC(args.n * args.n);
  va::fill_random(hA.data(), hA.size(), 1);
  va::fill_random(hB.data(), hB.size(), 2);

  float *A, *B, *C;
  CK(cudaMalloc(&A, bytes));
  CK(cudaMalloc(&B, bytes));
  CK(cudaMalloc(&C, bytes));
  CK(cudaMemcpy(A, hA.data(), bytes, cudaMemcpyHostToDevice));
  CK(cudaMemcpy(B, hB.data(), bytes, cudaMemcpyHostToDevice));

  if (args.variante == "cublas") {
    cublasCreate(&g_handle);
    // Standardmodus = FP32 ohne TF32-Tensorkerne, damit der Vergleich fair
    // bleibt (run_all.sh setzt zusätzlich NVIDIA_TF32_OVERRIDE=0).
    cublasSetMathMode(g_handle, CUBLAS_DEFAULT_MATH);
  }

  // Erster Lauf (Host-Uhr, inkl. Modul-Laden)
  va::Timer t0;
  fn(A, B, C, n);
  if (cudaGetLastError() != cudaSuccess) {
    std::fprintf(stderr, "Kernel startet nicht (Block zu groß?)\n");
    va::zeile(k, 0, NAN, 0.0, "GFLOPS", "startet_nicht");
    return 0;
  }
  CK(cudaDeviceSynchronize());
  const double first = t0.ms();
  va::zeile(k, 0, first, va::gemm_gflops(args.n, first), "GFLOPS", "erster_lauf");

  cudaEvent_t start, stop;
  CK(cudaEventCreate(&start));
  CK(cudaEventCreate(&stop));
  for (int r = 1; r <= args.reps; ++r) {
    CK(cudaEventRecord(start));
    fn(A, B, C, n);
    CK(cudaEventRecord(stop));
    CK(cudaEventSynchronize(stop));
    float ms = 0.0f;
    CK(cudaEventElapsedTime(&ms, start, stop));
    va::zeile(k, r, ms, va::gemm_gflops(args.n, ms), "GFLOPS", "ok");
  }

  CK(cudaMemcpy(hC.data(), C, bytes, cudaMemcpyDeviceToHost));
  const bool ok = va::verify_gemm(hA.data(), hB.data(), hC.data(), args.n);
  if (!ok) va::zeile(k, 0, NAN, 0.0, "GFLOPS", "falsch");

  if (args.variante == "cublas") cublasDestroy(g_handle);
  cudaFree(A);
  cudaFree(B);
  cudaFree(C);
  return ok ? 0 : 1;
}
