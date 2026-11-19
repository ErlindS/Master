// Reduktion Variante 6 – Buffer und Accessoren
//
// Gegenstück zu Stufe 2b der Matrixmultiplikation. Kernelstruktur wie in
// Variante 1 (private Teilsumme, dann ein Atomic je Work-Item), aber die Daten
// liegen in Buffern, der Zugriff läuft über Accessoren, und das Ergebnis kommt
// ohne ausdrückliches memcpy zurück.
//
// Zwei Programme aus derselben Quelle:
//   reduktion6_buffer         Buffer je Messung neu. Ein frisch angelegter
//                             Buffer liegt zunächst beim Host, die Laufzeit
//                             enthält also das Hinkopieren der n Werte -- anders
//                             als in den Varianten mit Gerätespeicher, wo die
//                             Eingabe vorab kopiert wird. Genau diese
//                             Buchführung hat auch Stufe 2b der Matmul.
//   reduktion6_buffer_wieder  Buffer einmal angelegt (-DBUFFER_WIEDER); die
//                             Eingabe liegt dann schon auf dem Gerät, übrig
//                             bleibt der Verwaltungsaufwand der Accessoren.
// Erst beide Zahlen zusammen trennen Datentransfer von Abstraktionskosten.

#include "sycl_rahmen.hpp"

#ifdef BUFFER_WIEDER
constexpr bool neu_je_lauf = false;
#else
constexpr bool neu_je_lauf = true;
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  sycl::queue q;
  const std::size_t G = rahmen::global_size(cfg.n, 256);

  auto launch = [&](sycl::buffer<float, 1>& bx, sycl::buffer<float, 1>& bs, std::size_t n) {
    q.submit([&](sycl::handler& h) {
      sycl::accessor x(bx, h, sycl::read_only);
      sycl::accessor s(bs, h, sycl::read_write);
      h.parallel_for(sycl::range<1>{G}, [=](sycl::id<1> id) {
        float p = 0.0f;
        for (std::size_t k = id[0]; k < n; k += G) p += x[k];
        sycl::atomic_ref<float, sycl::memory_order::relaxed, sycl::memory_scope::device,
                         sycl::access::address_space::global_space>
            a(s[0]);
        a.fetch_add(p);
      });
    });
  };

  const std::string name = neu_je_lauf ? "reduktion6_buffer" : "reduktion6_buffer_wieder";
  return rahmen::reduktion_buffer(name, q, cfg, launch, neu_je_lauf);
}
