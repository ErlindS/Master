// Stufe 3 – Work-Groups festlegen (nd_range), noch ohne lokalen Speicher
//
// Unterschied zu Stufe 2: Der Indexraum wird nicht mehr als einfache range,
// sondern als nd_range angegeben. Damit legt das Programm selbst fest, wie
// viele Work-Items eine Work-Group bilden (hier W x W). In Stufe 1 und 2
// hat das die Laufzeit entschieden.
//
// Der Rechenteil ist identisch mit Stufe 2: Jedes Work-Item liest weiterhin
// eine ganze Zeile von A und Spalte von B aus dem globalen Speicher. So
// lässt sich getrennt messen, was die Work-Group-Größe allein bringt, bevor
// Stufe 4 den lokalen Speicher hinzunimmt.
//
// Speicherverwaltung wie Stufe 2.
//
// Neue Konzepte: nd_range, nd_item, Work-Group, globale/lokale ID
//
// Work-Group-Größe beim Kompilieren ändern, z. B. -DWGSIZE=8 oder -DWGSIZE=32.

#include <sycl/sycl.hpp>
#include "common.hpp"

#ifndef WGSIZE
#define WGSIZE 16
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);
  constexpr std::size_t W = WGSIZE;

  sycl::queue q;
  const std::string dev = q.get_device().get_info<sycl::info::device::name>();

  std::vector<float> hA(n * n), hB(n * n), hC(n * n);
  bench::fill_random(hA.data(), n * n, 1);
  bench::fill_random(hB.data(), n * n, 2);

  float* A = sycl::malloc_device<float>(n * n, q);
  float* B = sycl::malloc_device<float>(n * n, q);
  float* C = sycl::malloc_device<float>(n * n, q);

  // Der globale Bereich muss ein Vielfaches der Work-Group-Größe sein
  const std::size_t np = (n + W - 1) / W * W;

  auto copy_in = [&] {
    q.memcpy(A, hA.data(), bytes);
    q.memcpy(B, hB.data(), bytes);
    q.wait();
  };
  auto kernel = [&] {
    q.parallel_for(
         sycl::nd_range<2>{{np, np}, {W, W}},  // global, lokal (Work-Group)
         [=](sycl::nd_item<2> it) {
           const std::size_t i = it.get_global_id(0);
           const std::size_t j = it.get_global_id(1);
           if (i >= n || j >= n) return;   // Work-Items außerhalb der Matrix
           float sum = 0.0f;
           for (std::size_t k = 0; k < n; ++k)
             sum += A[i * n + k] * B[k * n + j];
           C[i * n + j] = sum;
         })
        .wait();
  };
  auto copy_out = [&] { q.memcpy(hC.data(), C, bytes).wait(); };

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
  bench::report("3_ndrange_WG" + std::to_string(W), dev, n, first,
                bench::median(kernel_times), bench::median(transfer_times), ok);

  sycl::free(A, q);
  sycl::free(B, q);
  sycl::free(C, q);
  return ok ? 0 : 1;
}
