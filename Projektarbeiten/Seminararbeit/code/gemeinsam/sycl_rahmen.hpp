// sycl_rahmen.hpp – gemeinsamer Messrahmen der SYCL-Varianten
//
// Jedes Programm liefert nur seinen Kernel (als Funktion "launch"), der Rest
// – Speicher, Kopieren, Zeitmessung, Prüfung, Ausgabe – steht hier, damit sich
// die Varianten nur im Kernel unterscheiden.
#pragma once

#include <sycl/sycl.hpp>
#include "probleme.hpp"

namespace rahmen {

inline std::string device_name(const sycl::queue& q) {
  return q.get_device().get_info<sycl::info::device::name>();
}

// Stencil: launch(in, out) reicht EINEN Zeitschritt ein. Die Queue muss
// in-order sein, damit aufeinanderfolgende Zeitschritte nacheinander laufen,
// ohne dass der Host dazwischen wartet. Mit sync_each wartet der Host nach
// jedem Schritt (macht den Overhead pro Kernelstart sichtbar).
template <class Launch>
int stencil(const std::string& name, sycl::queue& q, const bench::Config& cfg, Launch launch,
            bool sync_each = false) {
  const std::size_t n = cfg.n;
  const std::size_t bytes = n * n * sizeof(float);
  const std::vector<float> start = bench::stencil::init(n);
  const std::vector<float> ref = bench::stencil::reference(n, ITER);

  float* a = sycl::malloc_device<float>(n * n, q);
  float* b = sycl::malloc_device<float>(n * n, q);

  auto reset = [&] {   // beide Puffer enthalten den festen Rand
    q.memcpy(a, start.data(), bytes);
    q.memcpy(b, start.data(), bytes);
    q.wait();
  };
  float* result = nullptr;
  auto run = [&] {
    float* in = a;
    float* out = b;
    for (int it = 0; it < ITER; ++it) {
      launch(in, out);
      if (sync_each) q.wait();
      std::swap(in, out);
    }
    q.wait();
    result = in;   // nach dem Tausch liegt das Ergebnis in "in"
  };

  reset();
  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    reset();
    bench::Timer t;
    run();
    times.push_back(t.ms());
  }

  std::vector<float> got(n * n);
  q.memcpy(got.data(), result, bytes).wait();
  const bool ok = bench::stencil::verify(got.data(), ref, n);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::stencil::bytes(n, ITER), ok);

  sycl::free(a, q);
  sycl::free(b, q);
  return ok ? 0 : 1;
}

// Reduktion: launch(x, sum, n) reicht die Reduktion ein, *sum ist vorher 0.
template <class Launch>
int reduktion(const std::string& name, sycl::queue& q, const bench::Config& cfg, Launch launch) {
  const std::size_t n = cfg.n;
  const std::vector<float> hx = bench::reduktion::init(n);
  const double ref = bench::reduktion::reference(hx);

  float* x = sycl::malloc_device<float>(n, q);
  float* sum = sycl::malloc_device<float>(1, q);
  q.memcpy(x, hx.data(), n * sizeof(float)).wait();   // Daten liegen vorab auf dem Gerät

  auto run = [&] {
    q.memset(sum, 0, sizeof(float)).wait();
    launch(x, sum, n);
    q.wait();
  };

  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    q.memset(sum, 0, sizeof(float)).wait();   // nicht mitgemessen
    bench::Timer t;
    launch(x, sum, n);
    q.wait();
    times.push_back(t.ms());
  }

  float got = 0.0f;
  q.memcpy(&got, sum, sizeof(float)).wait();
  const bool ok = bench::reduktion::verify(got, ref);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::reduktion::bytes(n), ok);

  sycl::free(x, q);
  sycl::free(sum, q);
  return ok ? 0 : 1;
}

// ---------------------------------------------------------------------------
// Gegenstücke zu den Speicher-Stufen der Matrixmultiplikation
//
// Die Fassungen oben entsprechen Stufe 2 (eigener Gerätespeicher, explizites
// Kopieren). Damit die Stufenfolge auch für die speichergebundenen
// Fallbeispiele vollständig ist, folgen hier dieselben Messabläufe mit
// gemeinsamem Speicher (Stufe 1) und mit Buffern (Stufe 2b). Gemessen wird
// jeweils genau dasselbe Zeitfenster, damit die Zahlen vergleichbar bleiben.
// ---------------------------------------------------------------------------

// Stufe 1: gemeinsamer Speicher (USM shared). Host und Gerät greifen auf
// dieselben Zeiger zu, das Verschieben der Daten übernimmt die Laufzeit.
template <class Launch>
int stencil_shared(const std::string& name, sycl::queue& q, const bench::Config& cfg,
                   Launch launch, bool sync_each = false) {
  const std::size_t n = cfg.n;
  const std::vector<float> start = bench::stencil::init(n);
  const std::vector<float> ref = bench::stencil::reference(n, ITER);

  float* a = sycl::malloc_shared<float>(n * n, q);
  float* b = sycl::malloc_shared<float>(n * n, q);
  if (!a || !b) {
    std::fprintf(stderr, "%s: kein gemeinsamer Speicher verfügbar\n", name.c_str());
    return 1;
  }

  auto reset = [&] {   // der Host schreibt direkt in den gemeinsamen Speicher
    std::copy(start.begin(), start.end(), a);
    std::copy(start.begin(), start.end(), b);
  };
  float* result = nullptr;
  auto run = [&] {
    float* in = a;
    float* out = b;
    for (int it = 0; it < ITER; ++it) {
      launch(in, out);
      if (sync_each) q.wait();
      std::swap(in, out);
    }
    q.wait();
    result = in;
  };

  reset();
  bench::Timer t0;
  run();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    reset();
    bench::Timer t;
    run();
    times.push_back(t.ms());
  }

  const bool ok = bench::stencil::verify(result, ref, n);   // direkt lesbar, kein Kopieren
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::stencil::bytes(n, ITER), ok);

  sycl::free(a, q);
  sycl::free(b, q);
  return ok ? 0 : 1;
}

// Stufe 2b: Buffer und Accessoren. launch(in, out) bekommt zwei Buffer und
// baut seine Command Group selbst; die Abhängigkeiten der ITER Zeitschritte
// ergeben sich allein aus den Accessoren, eine in-order-Queue ist unnötig.
//
// fresh = true: Buffer je Messung neu anlegen und zerstören. Dann enthält die
// Laufzeit auch das Anlegen, das Hinkopieren und das automatische
// Zurückkopieren -- dieselbe Buchführung wie bei Stufe 2b der
// Matrixmultiplikation. fresh = false: Buffer einmal anlegen und
// wiederverwenden, dann bleibt nur der Verwaltungsaufwand der Accessoren.
template <class Launch>
int stencil_buffer(const std::string& name, sycl::queue& q, const bench::Config& cfg,
                   Launch launch, bool fresh = true) {
  const std::size_t n = cfg.n;
  const std::vector<float> start = bench::stencil::init(n);
  const std::vector<float> ref = bench::stencil::reference(n, ITER);
  std::vector<float> ha(start), hb(start);

  // Die Zeitschritte wechseln zwischen beiden Puffern. Nach ITER Schritten
  // steht das Ergebnis bei geradem ITER in ha, bei ungeradem in hb. Bei
  // wiederverwendeten Buffern wird es am Ende nach "ergebnis" gelesen.
  const std::vector<float>& result = (ITER % 2 == 0) ? ha : hb;
  std::vector<float> ergebnis(fresh ? 0 : n * n);   // nur für wiederverwendete Buffer
  bool gelesen = false;

  auto reset = [&] {
    std::copy(start.begin(), start.end(), ha.begin());
    std::copy(start.begin(), start.end(), hb.begin());
  };
  auto schritte = [&](sycl::buffer<float, 1>& ba, sycl::buffer<float, 1>& bb) {
    for (int it = 0; it < ITER; ++it) {
      if (it % 2 == 0) launch(ba, bb);
      else launch(bb, ba);
    }
  };

  std::vector<double> times;
  double first = 0.0;

  if (fresh) {
    auto run = [&] {
      sycl::buffer<float, 1> ba(ha.data(), sycl::range<1>{n * n});
      sycl::buffer<float, 1> bb(hb.data(), sycl::range<1>{n * n});
      schritte(ba, bb);
    };   // beim Verlassen: warten und zurückkopieren
    reset();
    bench::Timer t0;
    run();
    first = t0.ms();
    for (int r = 0; r < cfg.reps; ++r) {
      reset();
      bench::Timer t;
      run();
      times.push_back(t.ms());
    }
  } else {
    sycl::buffer<float, 1> ba(ha.data(), sycl::range<1>{n * n});
    sycl::buffer<float, 1> bb(hb.data(), sycl::range<1>{n * n});
    auto reset_buf = [&] {   // Startwerte über Host-Accessoren, nicht mitgemessen
      sycl::host_accessor acc_a(ba);
      sycl::host_accessor acc_b(bb);
      for (std::size_t k = 0; k < n * n; ++k) acc_a[k] = acc_b[k] = start[k];
    };
    reset_buf();
    bench::Timer t0;
    schritte(ba, bb);
    q.wait();
    first = t0.ms();
    for (int r = 0; r < cfg.reps; ++r) {
      reset_buf();
      bench::Timer t;
      schritte(ba, bb);
      q.wait();
      times.push_back(t.ms());
    }
    // Solange die Buffer leben, darf der Host ha/hb nicht selbst lesen:
    // das Ergebnis kommt über einen Host-Accessor (nicht mitgemessen).
    sycl::buffer<float, 1>& bres = (ITER % 2 == 0) ? ba : bb;
    sycl::host_accessor acc(bres);
    for (std::size_t k = 0; k < n * n; ++k) ergebnis[k] = acc[k];
    gelesen = true;
  }

  const bool ok = bench::stencil::verify(gelesen ? ergebnis.data() : result.data(), ref, n);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::stencil::bytes(n, ITER), ok);
  return ok ? 0 : 1;
}

// Stufe 1 für die Reduktion: Eingabe und Summe im gemeinsamen Speicher.
template <class Launch>
int reduktion_shared(const std::string& name, sycl::queue& q, const bench::Config& cfg,
                     Launch launch) {
  const std::size_t n = cfg.n;
  const std::vector<float> hx = bench::reduktion::init(n);
  const double ref = bench::reduktion::reference(hx);

  float* x = sycl::malloc_shared<float>(n, q);
  float* sum = sycl::malloc_shared<float>(1, q);
  if (!x || !sum) {
    std::fprintf(stderr, "%s: kein gemeinsamer Speicher verfügbar\n", name.c_str());
    return 1;
  }
  std::copy(hx.begin(), hx.end(), x);   // Host schreibt direkt, kein memcpy

  *sum = 0.0f;
  bench::Timer t0;
  launch(x, sum, n);
  q.wait();
  const double first = t0.ms();

  std::vector<double> times;
  for (int r = 0; r < cfg.reps; ++r) {
    *sum = 0.0f;   // nicht mitgemessen
    bench::Timer t;
    launch(x, sum, n);
    q.wait();
    times.push_back(t.ms());
  }

  const bool ok = bench::reduktion::verify(*sum, ref);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::reduktion::bytes(n), ok);

  sycl::free(x, q);
  sycl::free(sum, q);
  return ok ? 0 : 1;
}

// Stufe 2b für die Reduktion: Eingabe und Summe als Buffer. Bei fresh = true
// enthält die Laufzeit das Hinkopieren der n Werte, denn ein neu angelegter
// Buffer liegt zunächst beim Host -- anders als in der Fassung mit
// Gerätespeicher, wo die Eingabe vorab kopiert wird.
template <class Launch>
int reduktion_buffer(const std::string& name, sycl::queue& q, const bench::Config& cfg,
                     Launch launch, bool fresh = true) {
  const std::size_t n = cfg.n;
  const std::vector<float> hx = bench::reduktion::init(n);
  const double ref = bench::reduktion::reference(hx);
  std::vector<float> x(hx);     // ein Buffer braucht einen nicht-konstanten Zeiger
  std::vector<float> hs(1, 0.0f);   // Hostspeicher der Summe
  float got = 0.0f;

  std::vector<double> times;
  double first = 0.0;

  if (fresh) {
    auto run = [&] {
      hs[0] = 0.0f;
      sycl::buffer<float, 1> bx(x.data(), sycl::range<1>{n});
      sycl::buffer<float, 1> bs(hs.data(), sycl::range<1>{1});
      launch(bx, bs, n);
    };   // beim Verlassen: warten und zurückkopieren
    bench::Timer t0;
    run();
    first = t0.ms();
    for (int r = 0; r < cfg.reps; ++r) {
      bench::Timer t;
      run();
      times.push_back(t.ms());
    }
    got = hs[0];
  } else {
    sycl::buffer<float, 1> bx(x.data(), sycl::range<1>{n});
    sycl::buffer<float, 1> bs(hs.data(), sycl::range<1>{1});
    auto reset = [&] { sycl::host_accessor acc(bs); acc[0] = 0.0f; };
    reset();
    bench::Timer t0;
    launch(bx, bs, n);
    q.wait();
    first = t0.ms();
    for (int r = 0; r < cfg.reps; ++r) {
      reset();   // nicht mitgemessen
      bench::Timer t;
      launch(bx, bs, n);
      q.wait();
      times.push_back(t.ms());
    }
    sycl::host_accessor acc(bs);   // Lesen erst über den Accessor, nicht über hs
    got = acc[0];
  }

  const bool ok = bench::reduktion::verify(got, ref);
  bench::report_bw(name, device_name(q), n, first, bench::median(times),
                   bench::reduktion::bytes(n), ok);
  return ok ? 0 : 1;
}

// Anzahl Work-Items für die Reduktionen mit eigener Schleife: höchstens
// 2^20, jedes summiert zuerst n / global Werte privat auf.
inline std::size_t global_size(std::size_t n, std::size_t wg) {
  const std::size_t g = std::min<std::size_t>(n, std::size_t(1) << 20);
  return (g + wg - 1) / wg * wg;
}

}  // namespace rahmen
