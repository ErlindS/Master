// Stufe 1 – Der erste SYCL-Kernel
//
// Kleinstmögliche Änderung gegenüber Stufe 0:
//   * eine Queue anlegen (sie wählt das Gerät, z. B. die GPU)
//   * Speicher mit malloc_shared anlegen (USM "shared"): derselbe Zeiger
//     ist auf Host und Gerät gültig, die Laufzeit verschiebt die Daten
//     automatisch dorthin, wo sie gebraucht werden
//   * die beiden äußeren Schleifen (i, j) durch parallel_for ersetzen –
//     jedes C[i][j] wird von einem eigenen Work-Item berechnet
//
// Die innere Schleife (k) bleibt unverändert im Kernel.
//
// Neue Konzepte: queue, device, kernel, parallel_for, range/id, USM shared

#include <sycl/sycl.hpp>
#include "common.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;

  // 1) Queue: Verbindung zu einem Gerät. Ohne Angabe nimmt SYCL das
  //    "beste" verfügbare Gerät (meist die GPU).
  sycl::queue q;
  const std::string dev = q.get_device().get_info<sycl::info::device::name>();

  // 2) Gemeinsamer Speicher (Unified Shared Memory, Variante "shared")
  float* A = sycl::malloc_shared<float>(n * n, q);
  float* B = sycl::malloc_shared<float>(n * n, q);
  float* C = sycl::malloc_shared<float>(n * n, q);
  bench::fill_random(A, n * n, 1);
  bench::fill_random(B, n * n, 2);

  // 3) Kernel: ersetzt die i- und j-Schleife aus Stufe 0
  auto run = [&] {
    q.parallel_for(sycl::range<2>{n, n}, [=](sycl::id<2> idx) {
       const std::size_t i = idx[0];
       const std::size_t j = idx[1];
       float sum = 0.0f;
       for (std::size_t k = 0; k < n; ++k)
         sum += A[i * n + k] * B[k * n + j];
       C[i * n + j] = sum;
     }).wait();  // auf das Ende des Kernels warten
  };

  // Erster Lauf: enthält ggf. JIT-Kompilierung des Kernels und das erste
  // Verschieben der Daten auf das Gerät -> separat ausweisen!
  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    bench::Timer t;
    run();
    times.push_back(t.ms());
  }

  // Zugriff auf C vom Host aus: mit malloc_shared einfach möglich
  const bool ok = bench::verify(A, B, C, n);
  bench::report("1_usm_shared", dev, n, first, bench::median(times), 0.0, ok);

  sycl::free(A, q);
  sycl::free(B, q);
  sycl::free(C, q);
  return ok ? 0 : 1;
}
