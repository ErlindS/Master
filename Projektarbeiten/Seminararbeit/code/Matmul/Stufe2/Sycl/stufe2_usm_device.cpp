// Stufe 2 – Speicher selbst verwalten
//
// Unterschied zu Stufe 1: Statt malloc_shared (automatisches Verschieben)
// wird der Speicher explizit getrennt:
//   * Host-Speicher (normales std::vector) für Initialisierung und Prüfung
//   * Geräte-Speicher (malloc_device), nur auf dem Gerät gültig
//   * Kopieren mit q.memcpy, Host -> Gerät und Gerät -> Host
//
// Der Kernel selbst ist identisch zu Stufe 1.
//
// Ziel für die Präsentation: Datentransfer wird sichtbar und messbar.
// Gerade bei kleinen Matrizen kostet das Kopieren oft mehr als die Rechnung.
//
// Neue Konzepte: malloc_device, memcpy, Trennung Host-/Geräte-Speicher

#include <sycl/sycl.hpp>
#include "common.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);

  sycl::queue q;
  const std::string dev = q.get_device().get_info<sycl::info::device::name>();

  // Host-Seite
  std::vector<float> hA(n * n), hB(n * n), hC(n * n);
  bench::fill_random(hA.data(), n * n, 1);
  bench::fill_random(hB.data(), n * n, 2);

  // Geräte-Seite
  float* A = sycl::malloc_device<float>(n * n, q);
  float* B = sycl::malloc_device<float>(n * n, q);
  float* C = sycl::malloc_device<float>(n * n, q);

  auto copy_in = [&] {
    q.memcpy(A, hA.data(), bytes);
    q.memcpy(B, hB.data(), bytes);
    q.wait();
  };
  auto kernel = [&] {
    q.parallel_for(sycl::range<2>{n, n}, [=](sycl::id<2> idx) {
       const std::size_t i = idx[0];
       const std::size_t j = idx[1];
       float sum = 0.0f;
       for (std::size_t k = 0; k < n; ++k)
         sum += A[i * n + k] * B[k * n + j];
       C[i * n + j] = sum;
     }).wait();
  };
  auto copy_out = [&] { q.memcpy(hC.data(), C, bytes).wait(); };

  // Erster Lauf komplett (Kopieren + Kernel inkl. JIT + Zurückkopieren)
  bench::Timer t0;
  copy_in();
  kernel();
  copy_out();
  const double first = t0.ms();

  // Messläufe: Kernel und Transfer getrennt erfassen
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
  bench::report("2_usm_device", dev, n, first, bench::median(kernel_times),
                bench::median(transfer_times), ok);

  sycl::free(A, q);
  sycl::free(B, q);
  sycl::free(C, q);
  return ok ? 0 : 1;
}
