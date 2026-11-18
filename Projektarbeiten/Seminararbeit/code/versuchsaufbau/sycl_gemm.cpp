// sycl_gemm.cpp – Matrixmultiplikation in SYCL, alle Varianten in einem Programm
//
//   ./sycl_gemm_dpcpp <variante> <n> [wiederholungen]
//
// Varianten:
//   naiv     Kernel aus Stufe 1/2 (range, ein Work-Item pro C[i][j])
//   tile8 … tile64
//            Kachelung aus Stufe 3 mit fester Kachelgröße T
//   auto     gerätebewusst: nur Kachelgrößen, die laut get_info auf das Gerät
//            passen, werden kurz ausprobiert, die schnellste wird gemessen
//
// Gemessen wird die Kernel-Zeit über SYCL-Events (enable_profiling), also
// ohne Overhead der Laufzeit auf dem Host. Der erste Lauf (inkl.
// JIT-Kompilierung) wird separat mit der Host-Uhr erfasst. Bei "naiv" wird
// zusätzlich der Datentransfer (Host -> Gerät -> Host) gemessen.

#include <sycl/sycl.hpp>

#include <exception>
#include <limits>

#include "common.hpp"

#ifndef IMPL_NAME
#define IMPL_NAME "sycl"
#endif

namespace {

double event_ms(const sycl::event& e) {
  const auto s = e.get_profiling_info<sycl::info::event_profiling::command_start>();
  const auto t = e.get_profiling_info<sycl::info::event_profiling::command_end>();
  return double(t - s) * 1e-6;
}

sycl::event gemm_naiv(sycl::queue& q, const float* A, const float* B, float* C,
                      std::size_t n) {
  return q.parallel_for(sycl::range<2>{n, n}, [=](sycl::id<2> idx) {
    const std::size_t i = idx[0], j = idx[1];
    float sum = 0.0f;
    for (std::size_t k = 0; k < n; ++k) sum += A[i * n + k] * B[k * n + j];
    C[i * n + j] = sum;
  });
}

// KERNEL-BEGIN tiled
template <std::size_t T>
sycl::event gemm_tile(sycl::queue& q, const float* A, const float* B, float* C,
                      std::size_t n) {
  const std::size_t np = (n + T - 1) / T * T;  // auf Vielfaches von T aufrunden
  return q.submit([&](sycl::handler& h) {
    sycl::local_accessor<float, 2> tA(sycl::range<2>{T, T}, h);
    sycl::local_accessor<float, 2> tB(sycl::range<2>{T, T}, h);
    h.parallel_for(sycl::nd_range<2>{{np, np}, {T, T}}, [=](sycl::nd_item<2> it) {
      const std::size_t i = it.get_global_id(0), j = it.get_global_id(1);
      const std::size_t li = it.get_local_id(0), lj = it.get_local_id(1);
      float sum = 0.0f;
      for (std::size_t t = 0; t < n; t += T) {
        tA[li][lj] = (i < n && t + lj < n) ? A[i * n + t + lj] : 0.0f;
        tB[li][lj] = (t + li < n && j < n) ? B[(t + li) * n + j] : 0.0f;
        sycl::group_barrier(it.get_group());
        for (std::size_t k = 0; k < T; ++k) sum += tA[li][k] * tB[k][lj];
        sycl::group_barrier(it.get_group());
      }
      if (i < n && j < n) C[i * n + j] = sum;
    });
  });
}
// KERNEL-END tiled

using GemmFn = sycl::event (*)(sycl::queue&, const float*, const float*, float*,
                               std::size_t);

struct Kandidat {
  std::size_t T;
  GemmFn fn;
};

const Kandidat kKandidaten[] = {
    {8, gemm_tile<8>}, {16, gemm_tile<16>}, {32, gemm_tile<32>}, {64, gemm_tile<64>}};

// Passt eine Work-Group von T x T mit zwei Kacheln in den lokalen Speicher?
bool passt(const sycl::device& d, std::size_t T) {
  return T * T <= d.get_info<sycl::info::device::max_work_group_size>() &&
         2 * T * T * sizeof(float) <= d.get_info<sycl::info::device::local_mem_size>();
}

}  // namespace

int main(int argc, char** argv) {
  const auto args = va::parse_args(argc, argv, "naiv|tile8|tile16|tile32|tile64|auto");
  const std::size_t n = args.n;
  const std::size_t bytes = n * n * sizeof(float);

  // Asynchrone Fehler (z. B. beim Kernel-Start) als Ausnahme weiterreichen.
  auto async_handler = [](sycl::exception_list l) {
    for (auto& e : l) std::rethrow_exception(e);
  };
  sycl::queue q{sycl::default_selector_v, async_handler,
                sycl::property_list{sycl::property::queue::enable_profiling{}}};
  const sycl::device dev = q.get_device();

  va::Kontext k{IMPL_NAME,
                dev.is_gpu() ? "gpu" : (dev.is_cpu() ? "cpu" : "andere"),
                dev.get_info<sycl::info::device::name>(),
                "gemm",
                args.variante,
                n};

  std::vector<float> hA(n * n), hB(n * n), hC(n * n);
  va::fill_random(hA.data(), n * n, 1);
  va::fill_random(hB.data(), n * n, 2);

  float* A = sycl::malloc_device<float>(n * n, q);
  float* B = sycl::malloc_device<float>(n * n, q);
  float* C = sycl::malloc_device<float>(n * n, q);
  q.memcpy(A, hA.data(), bytes);
  q.memcpy(B, hB.data(), bytes);
  q.wait_and_throw();

  GemmFn fn = nullptr;
  if (args.variante == "naiv") {
    fn = gemm_naiv;
  } else if (args.variante == "auto") {
    // Gerätebewusste Auswahl: ungeeignete Kachelgrößen werden gar nicht erst
    // gestartet, die übrigen kurz gemessen (Minimum aus 3 Läufen).
    va::Timer tt;
    double best_ms = std::numeric_limits<double>::infinity();
    std::size_t best_T = 0;
    for (const auto& c : kKandidaten) {
      if (!passt(dev, c.T)) continue;
      try {
        c.fn(q, A, B, C, n).wait_and_throw();  // Aufwärmen inkl. JIT
        for (int r = 0; r < 3; ++r) {
          auto e = c.fn(q, A, B, C, n);
          e.wait_and_throw();
          const double ms = event_ms(e);
          if (ms < best_ms) best_ms = ms, best_T = c.T;
        }
      } catch (const sycl::exception&) {
        continue;
      }
    }
    if (best_T == 0) {
      va::zeile(k, 0, NAN, 0.0, "GFLOPS", "startet_nicht");
      return 1;
    }
    for (const auto& c : kKandidaten)
      if (c.T == best_T) fn = c.fn;
    k.variante = "auto_T" + std::to_string(best_T);
    va::zeile(k, 0, tt.ms(), double(best_T), "T", "tuning");
  } else {
    for (const auto& c : kKandidaten)
      if (args.variante == "tile" + std::to_string(c.T)) fn = c.fn;
    if (!fn) {
      std::fprintf(stderr, "unbekannte Variante: %s\n", args.variante.c_str());
      return 2;
    }
  }

  try {
    va::Timer t0;
    fn(q, A, B, C, n).wait_and_throw();
    const double first = t0.ms();
    va::zeile(k, 0, first, va::gemm_gflops(n, first), "GFLOPS", "erster_lauf");

    for (int r = 1; r <= args.reps; ++r) {
      auto e = fn(q, A, B, C, n);
      e.wait_and_throw();
      const double ms = event_ms(e);
      va::zeile(k, r, ms, va::gemm_gflops(n, ms), "GFLOPS", "ok");
    }
  } catch (const sycl::exception& ex) {
    std::fprintf(stderr, "Kernel startet nicht: %s\n", ex.what());
    va::zeile(k, 0, NAN, 0.0, "GFLOPS", "startet_nicht");
    return 0;  // erwartetes Ergebnis (z. B. tile64 auf NVIDIA), kein Fehler
  }

  q.memcpy(hC.data(), C, bytes).wait_and_throw();
  const bool ok = va::verify_gemm(hA.data(), hB.data(), hC.data(), n);
  if (!ok) va::zeile(k, 0, NAN, 0.0, "GFLOPS", "falsch");

  // Datentransfer einmal pro Konfiguration genügt (unabhängig vom Kernel).
  if (args.variante == "naiv") {
    va::Kontext kt = k;
    kt.kernel = "transfer";
    for (int r = 1; r <= args.reps; ++r) {
      auto e1 = q.memcpy(A, hA.data(), bytes);
      auto e2 = q.memcpy(B, hB.data(), bytes);
      auto e3 = q.memcpy(hC.data(), C, bytes);
      q.wait_and_throw();
      const double ms = event_ms(e1) + event_ms(e2) + event_ms(e3);
      va::zeile(kt, r, ms, 3.0 * double(bytes) / (ms * 1e6), "GB/s", "ok");
    }
  }

  sycl::free(A, q);
  sycl::free(B, q);
  sycl::free(C, q);
  return ok ? 0 : 1;
}
