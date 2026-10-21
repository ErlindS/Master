// Stufe 3 – Hardwarenah optimieren: Work-Groups und Local Memory (Tiling)
//
// Problem der Stufen 1 und 2: Jedes Work-Item liest seine komplette Zeile
// von A und Spalte von B aus dem langsamen globalen Speicher. Benachbarte
// Work-Items lesen dabei immer wieder dieselben Werte.
//
// Idee: Die Matrix wird in Kacheln (Tiles) der Größe TILE x TILE zerlegt.
//   * Work-Items werden zu Work-Groups von TILE x TILE zusammengefasst
//   * Jede Work-Group lädt gemeinsam eine Kachel von A und eine von B in
//     den schnellen Local Memory (auf der GPU: Shared Memory des Rechenblocks)
//   * Barriere: warten, bis alle Werte geladen sind
//   * Jedes Work-Item rechnet mit den Kacheldaten weiter
//   * Barriere, dann die nächste Kachel laden
// Dadurch wird jeder Wert aus dem globalen Speicher nur noch einmal pro
// Work-Group statt einmal pro Work-Item gelesen.
//
// Speicherverwaltung wie Stufe 2 – geändert hat sich nur der Kernel.
//
// Neue Konzepte: nd_range, nd_item, Work-Group, local_accessor (Local
// Memory), group_barrier
//
// Kachelgröße beim Kompilieren ändern, z. B. -DTILE=8 oder -DTILE=32.
// Die beste Wahl hängt vom Gerät ab (-> Performance-Portabilität!).

#include <sycl/sycl.hpp>
#include "common.hpp"

#ifndef TILE
#define TILE 16
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);
  constexpr std::size_t T = TILE;

  sycl::queue q;
  const std::string dev = q.get_device().get_info<sycl::info::device::name>();

  std::vector<float> hA(n * n), hB(n * n), hC(n * n);
  bench::fill_random(hA.data(), n * n, 1);
  bench::fill_random(hB.data(), n * n, 2);

  float* A = sycl::malloc_device<float>(n * n, q);
  float* B = sycl::malloc_device<float>(n * n, q);
  float* C = sycl::malloc_device<float>(n * n, q);

  // Globaler Bereich auf ein Vielfaches der Kachelgröße aufrunden, damit
  // auch n, die nicht durch TILE teilbar sind, funktionieren.
  const std::size_t np = (n + T - 1) / T * T;

  auto copy_in = [&] {
    q.memcpy(A, hA.data(), bytes);
    q.memcpy(B, hB.data(), bytes);
    q.wait();
  };

  auto kernel = [&] {
    q.submit([&](sycl::handler& h) {
       // Local Memory: eine Kachel von A und eine von B pro Work-Group
       sycl::local_accessor<float, 2> tileA({T, T}, h);
       sycl::local_accessor<float, 2> tileB({T, T}, h);

       h.parallel_for(
           sycl::nd_range<2>{{np, np}, {T, T}},  // global, lokal (Work-Group)
           [=](sycl::nd_item<2> it) {
             const std::size_t i = it.get_global_id(0);   // Zeile in C
             const std::size_t j = it.get_global_id(1);   // Spalte in C
             const std::size_t li = it.get_local_id(0);   // Position in
             const std::size_t lj = it.get_local_id(1);   // der Kachel

             float sum = 0.0f;
             for (std::size_t t = 0; t < n; t += T) {
               // gemeinsam eine Kachel laden (außerhalb der Matrix: 0)
               tileA[li][lj] = (i < n && t + lj < n) ? A[i * n + t + lj] : 0.0f;
               tileB[li][lj] = (t + li < n && j < n) ? B[(t + li) * n + j] : 0.0f;
               sycl::group_barrier(it.get_group());

               // mit den Daten aus dem schnellen Local Memory rechnen
               for (std::size_t k = 0; k < T; ++k)
                 sum += tileA[li][k] * tileB[k][lj];
               sycl::group_barrier(it.get_group());
             }
             if (i < n && j < n) C[i * n + j] = sum;
           });
     }).wait();
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
  bench::report("3_tiling_T" + std::to_string(T), dev, n, first,
                bench::median(kernel_times), bench::median(transfer_times), ok);

  sycl::free(A, q);
  sycl::free(B, q);
  sycl::free(C, q);
  return ok ? 0 : 1;
}
