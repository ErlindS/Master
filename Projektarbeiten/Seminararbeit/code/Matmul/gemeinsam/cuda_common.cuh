// cuda_common.cuh – Hilfen für die nativen CUDA-Vergleichsprogramme
//
// Messung, Prüfung und CSV-Ausgabe kommen unverändert aus ../common.hpp,
// damit CUDA- und SYCL-Ergebnisse direkt vergleichbar sind.
#pragma once

#include <cuda_runtime.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "common.hpp"   // code/gemeinsam, per -I

// Jeden CUDA-Aufruf prüfen. CUDA meldet Fehler über Rückgabewerte statt
// über Ausnahmen (Gegenstück zu sycl::exception in Stufe 5).
#define CUDA_CHECK(call)                                                        \
  do {                                                                          \
    const cudaError_t err_ = (call);                                            \
    if (err_ != cudaSuccess) {                                                  \
      std::fprintf(stderr, "CUDA-Fehler %s in %s:%d: %s\n", #call, __FILE__,    \
                   __LINE__, cudaGetErrorString(err_));                         \
      std::exit(2);                                                             \
    }                                                                           \
  } while (0)

// Kernelstart prüfen: Konfigurationsfehler (z. B. zu große Blöcke) meldet
// cudaGetLastError, Laufzeitfehler im Kernel erst die Synchronisation.
#define CUDA_CHECK_LAUNCH()                                                     \
  do {                                                                          \
    CUDA_CHECK(cudaGetLastError());                                             \
    CUDA_CHECK(cudaDeviceSynchronize());                                        \
  } while (0)

inline std::string cuda_device_name() {
  int dev = 0;
  CUDA_CHECK(cudaGetDevice(&dev));
  cudaDeviceProp p{};
  CUDA_CHECK(cudaGetDeviceProperties(&p, dev));
  return p.name;
}
