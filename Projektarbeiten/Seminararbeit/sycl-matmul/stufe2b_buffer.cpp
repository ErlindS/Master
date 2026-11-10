// Stufe 2b – Variante von Stufe 2: Buffer und Accessoren statt USM
//
// Gleicher Kernel wie in Stufe 1 und 2, aber die Daten werden nicht über
// Zeiger, sondern über Buffer verwaltet:
//   * Ein Buffer kapselt die Daten eines Host-Felds
//   * Der Kernel greift nur über Accessoren zu, die angeben, ob gelesen oder
//     geschrieben wird (read_only, write_only)
//   * Aus diesen Angaben leitet die Laufzeit selbst ab, wann kopiert werden
//     muss und welche Aufträge voneinander abhängen. Es gibt weder memcpy
//     noch ein explizites wait.
//   * Wird der Buffer zerstört, schreibt die Laufzeit das Ergebnis in das
//     Host-Feld zurück und wartet darauf.
//
// Preis der Abstraktion: Kopieren und Rechnen lassen sich nicht mehr
// getrennt messen. median_ms enthält hier Transfer UND Kernel.
//
// Neue Konzepte: buffer, accessor, Zugriffsmodi, implizite Abhängigkeiten

#include <sycl/sycl.hpp>
#include "common.hpp"

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;

  sycl::queue q;
  const std::string dev = q.get_device().get_info<sycl::info::device::name>();

  std::vector<float> hA(n * n), hB(n * n), hC(n * n);
  bench::fill_random(hA.data(), n * n, 1);
  bench::fill_random(hB.data(), n * n, 2);

  // Ein vollständiger Durchlauf: Buffer anlegen, Kernel einreichen, beim
  // Verlassen des Blocks schreiben die Buffer-Destruktoren C zurück.
  auto lauf = [&] {
    sycl::buffer<float, 1> bA(hA.data(), sycl::range<1>{n * n});
    sycl::buffer<float, 1> bB(hB.data(), sycl::range<1>{n * n});
    sycl::buffer<float, 1> bC(hC.data(), sycl::range<1>{n * n});

    q.submit([&](sycl::handler& h) {
      sycl::accessor A(bA, h, sycl::read_only);
      sycl::accessor B(bB, h, sycl::read_only);
      sycl::accessor C(bC, h, sycl::write_only, sycl::no_init);

      h.parallel_for(sycl::range<2>{n, n}, [=](sycl::id<2> idx) {
        const std::size_t i = idx[0];
        const std::size_t j = idx[1];
        float sum = 0.0f;
        for (std::size_t k = 0; k < n; ++k)
          sum += A[i * n + k] * B[k * n + j];
        C[i * n + j] = sum;
      });
    });
  };  // <- hier: Kopie von C zum Host, Host wartet

  bench::Timer t0;
  lauf();
  const double first = t0.ms();

  // Messläufe: nur die Gesamtzeit (Transfer + Kernel) ist messbar
  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    bench::Timer t;
    lauf();
    times.push_back(t.ms());
  }

  const bool ok = bench::verify(hA.data(), hB.data(), hC.data(), n);
  bench::report("2b_buffer", dev, n, first, bench::median(times), 0.0, ok);
  return ok ? 0 : 1;
}
