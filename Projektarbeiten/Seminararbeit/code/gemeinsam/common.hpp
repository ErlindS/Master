// common.hpp – gemeinsame Hilfsfunktionen für alle Stufen
//
// Enthält: Kommandozeile lesen, Matrizen befüllen, Zeitmessung,
// Ergebnisprüfung und eine einheitliche CSV-Ausgabe, damit die
// Ergebnisse aller Stufen direkt vergleichbar sind.
#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

namespace bench {

// Einstellungen aus der Kommandozeile:  ./stufeX <n> [wiederholungen]
struct Config {
  std::size_t n = 1024;   // Matrixgröße n x n
  int reps = 5;           // Messwiederholungen (nach dem Aufwärmlauf)
};

inline Config parse_args(int argc, char** argv) {
  Config c;
  if (argc > 1) c.n = std::strtoul(argv[1], nullptr, 10);
  if (argc > 2) c.reps = std::atoi(argv[2]);
  if (c.n == 0 || c.reps < 1) {
    std::fprintf(stderr, "Aufruf: %s <n> [wiederholungen]\n", argv[0]);
    std::exit(1);
  }
  return c;
}

// Matrizen mit reproduzierbaren Zufallswerten im Bereich [-1, 1] füllen.
inline void fill_random(float* m, std::size_t count, unsigned seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  for (std::size_t i = 0; i < count; ++i) m[i] = dist(gen);
}

// Einfache Stoppuhr in Millisekunden.
class Timer {
 public:
  Timer() : start_(clock::now()) {}
  double ms() const {
    return std::chrono::duration<double, std::milli>(clock::now() - start_).count();
  }
 private:
  using clock = std::chrono::steady_clock;
  clock::time_point start_;
};

inline double median(std::vector<double> v) {
  std::sort(v.begin(), v.end());
  const std::size_t m = v.size() / 2;
  return v.size() % 2 ? v[m] : 0.5 * (v[m - 1] + v[m]);
}

// Ergebnisprüfung: 64 zufällige Einträge von C werden in double-Genauigkeit
// nachgerechnet. Das kostet nur O(64 * n) und funktioniert auch für große n.
inline bool verify(const float* A, const float* B, const float* C, std::size_t n) {
  std::mt19937 gen(42);
  std::uniform_int_distribution<std::size_t> idx(0, n - 1);
  for (int s = 0; s < 64; ++s) {
    const std::size_t i = idx(gen), j = idx(gen);
    double ref = 0.0;
    for (std::size_t k = 0; k < n; ++k) ref += double(A[i * n + k]) * double(B[k * n + j]);
    const double got = C[i * n + j];
    const double tol = 1e-3 * std::max(1.0, std::sqrt(double(n)));
    if (std::fabs(got - ref) > tol) {
      std::fprintf(stderr, "FEHLER bei C[%zu][%zu]: %f statt %f\n", i, j, got, ref);
      return false;
    }
  }
  return true;
}

// Einheitliche Ergebniszeile (CSV), damit sich alle Stufen in eine Tabelle
// schreiben lassen:
//   stufe;geraet;n;erster_lauf_ms;median_ms;transfer_ms;gflops;korrekt
inline void report(const std::string& stage, const std::string& device, std::size_t n,
                   double first_ms, double median_ms, double transfer_ms, bool ok) {
  const double gflops = 2.0 * double(n) * double(n) * double(n) / (median_ms * 1e6);
  std::printf("%s;%s;%zu;%.3f;%.3f;%.3f;%.2f;%s\n", stage.c_str(), device.c_str(), n,
              first_ms, median_ms, transfer_ms, gflops, ok ? "ja" : "NEIN");
}

}  // namespace bench
