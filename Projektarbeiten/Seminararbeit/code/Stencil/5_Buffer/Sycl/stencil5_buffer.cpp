// Stencil Variante 5 – Buffer und Accessoren
//
// Gegenstück zu Stufe 2b der Matrixmultiplikation: Statt Zeiger und
// ausdrücklichem Kopieren beschreiben Buffer die Daten und Accessoren den
// Zugriff. Die Reihenfolge der ITER Zeitschritte ergibt sich allein daraus,
// dass jeder Schritt den Puffer liest, den der vorige geschrieben hat -- die
// Laufzeit baut daraus selbst einen Abhängigkeitsgraphen. Eine in-order-Queue
// ist deshalb nicht nötig, anders als in Variante 1.
//
// Der Ausgabe-Accessor ist bewusst nur write_only ohne no_init: Ein Zeitschritt
// schreibt ausschließlich die inneren Punkte, der feste Rand im Zielpuffer muss
// erhalten bleiben.
//
// Zwei Programme aus derselben Quelle:
//   stencil5_buffer         Buffer je Messung neu (enthält Anlegen, Hin- und
//                           Zurückkopieren -- wie Stufe 2b der Matmul)
//   stencil5_buffer_wieder  Buffer einmal angelegt (-DBUFFER_WIEDER); dann
//                           bleibt nur der Verwaltungsaufwand der Accessoren
// Erst die Differenz der beiden trennt Datentransfer von Abstraktionskosten.

#include "sycl_rahmen.hpp"

#ifdef BUFFER_WIEDER
constexpr bool neu_je_lauf = false;
#else
constexpr bool neu_je_lauf = true;
#endif

int main(int argc, char** argv) {
  const auto cfg = bench::parse_args(argc, argv);
  const std::size_t n = cfg.n;
  sycl::queue q;   // keine in-order-Queue: die Accessoren ordnen die Schritte

  auto launch = [&](sycl::buffer<float, 1>& bin, sycl::buffer<float, 1>& bout) {
    q.submit([&](sycl::handler& h) {
      sycl::accessor in(bin, h, sycl::read_only);
      sycl::accessor out(bout, h, sycl::write_only);   // ohne no_init: Rand bleibt
      h.parallel_for(sycl::range<2>{n - 2, n - 2}, [=](sycl::id<2> idx) {
        const std::size_t i = idx[0] + 1, j = idx[1] + 1;
        out[i * n + j] = bench::stencil::update(in[(i - 1) * n + j], in[(i + 1) * n + j],
                                               in[i * n + j - 1], in[i * n + j + 1]);
      });
    });
  };

  const std::string name = neu_je_lauf ? "stencil5_buffer" : "stencil5_buffer_wieder";
  return rahmen::stencil_buffer(name, q, cfg, launch, neu_je_lauf);
}
