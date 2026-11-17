// Obergrenze: cub::DeviceReduce::Sum (NVIDIAs optimierte Bibliothek, Teil
// des CUDA-Toolkits). Gegenstück zu reduktion4_builtin – in beiden Fällen
// entscheidet die Bibliothek bzw. Implementierung, wie reduziert wird.

#include <cub/device/device_reduce.cuh>
#include "cuda_rahmen.cuh"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);

  // Zwischenspeicher einmal vorab anlegen (nicht mitgemessen)
  void* tmp = nullptr;
  std::size_t tmp_bytes = 0;
  CUDA_CHECK(cub::DeviceReduce::Sum(tmp, tmp_bytes, static_cast<const float*>(nullptr),
                                    static_cast<float*>(nullptr), cfg.n));
  CUDA_CHECK(cudaMalloc(&tmp, tmp_bytes));

  auto launch = [&](const float* x, float* sum, std::size_t n) {
    CUDA_CHECK(cub::DeviceReduce::Sum(tmp, tmp_bytes, x, sum, n));
  };
  const int rc = rahmen::reduktion("cuda_reduktion_cub", cfg, launch);
  CUDA_CHECK(cudaFree(tmp));
  return rc;
}
