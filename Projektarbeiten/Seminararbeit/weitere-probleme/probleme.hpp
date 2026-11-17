// probleme.hpp – Host-Teil der beiden zusätzlichen Fallbeispiele
//
// Wird von SYCL- und CUDA-Programmen gleichermaßen genutzt: Daten erzeugen,
// Referenzergebnis auf dem Host berechnen, prüfen und als CSV ausgeben.
// Zeitmessung, Median und Kommandozeile kommen aus ../sycl-matmul/common.hpp.
//
// Ausgabeformat (eine Zeile je Programmlauf):
//   variante;geraet;n;erster_lauf_ms;median_ms;gb_s;korrekt
// gb_s ist die effektive Speicherbandbreite: mindestens nötige Bytes / Zeit.
#pragma once

#include <cmath>
#include <utility>
#include "../sycl-matmul/common.hpp"

#ifndef ITER
#define ITER 100   // Zeitschritte des Stencils je Messung
#endif

namespace bench {

inline void report_bw(const std::string& variant, const std::string& device, std::size_t n,
                      double first_ms, double median_ms, double bytes, bool ok) {
  std::printf("%s;%s;%zu;%.3f;%.3f;%.2f;%s\n", variant.c_str(), device.c_str(), n, first_ms,
              median_ms, bytes / (median_ms * 1e6), ok ? "ja" : "NEIN");
}

// ---------------------------------------------------------------------------
// Fallbeispiel 2: 2D-Wärmeleitung, Jacobi-Verfahren mit 5-Punkt-Stencil
//
// n x n Gitter, Rand fest (obere Kante 1, sonst 0), Inneres startet bei 0.
// Je Zeitschritt: neu[i][j] = 0,25 * (oben + unten + links + rechts).
// Je Punkt 4 Additionen/Multiplikationen auf 5 gelesene und 1 geschriebenen
// Wert -> klar speichergebunden (Gegenstück zur Matrixmultiplikation).
// ---------------------------------------------------------------------------
namespace stencil {

inline std::vector<float> init(std::size_t n) {
  std::vector<float> g(n * n, 0.0f);
  for (std::size_t j = 0; j < n; ++j) g[j] = 1.0f;   // obere Kante heiß
  return g;
}

// Ein Zeitschritt. Die Additionsreihenfolge ist in allen Kerneln gleich,
// damit Host und Gerät bitgenau dasselbe rechnen können.
inline float update(float up, float down, float left, float right) {
  return 0.25f * (((up + down) + left) + right);
}

inline std::vector<float> reference(std::size_t n, int iters) {
  std::vector<float> a = init(n), b = a;
  for (int it = 0; it < iters; ++it) {
    for (std::size_t i = 1; i + 1 < n; ++i)
      for (std::size_t j = 1; j + 1 < n; ++j)
        b[i * n + j] = update(a[(i - 1) * n + j], a[(i + 1) * n + j], a[i * n + j - 1],
                              a[i * n + j + 1]);
    std::swap(a, b);
  }
  return a;
}

inline bool verify(const float* got, const std::vector<float>& ref, std::size_t n) {
  for (std::size_t k = 0; k < n * n; ++k)
    if (std::fabs(got[k] - ref[k]) > 1e-5f) {
      std::fprintf(stderr, "FEHLER bei (%zu,%zu): %f statt %f\n", k / n, k % n, got[k], ref[k]);
      return false;
    }
  return true;
}

// Mindestens nötiger Speicherverkehr: jeder Punkt einmal lesen, einmal schreiben
inline double bytes(std::size_t n, int iters) { return 2.0 * iters * double(n) * n * sizeof(float); }

}  // namespace stencil

// ---------------------------------------------------------------------------
// Fallbeispiel 3: Reduktion (Summe eines Vektors mit n Elementen)
//
// Eine Addition je gelesenem Wert, dafür Zusammenarbeit über alle
// Work-Items hinweg -> bandbreiten- und synchronisationsgebunden.
// ---------------------------------------------------------------------------
namespace reduktion {

inline std::vector<float> init(std::size_t n) {
  std::vector<float> x(n);
  std::mt19937 gen(3);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for (auto& v : x) v = dist(gen);
  return x;
}

inline double reference(const std::vector<float>& x) {
  double s = 0.0;
  for (float v : x) s += v;
  return s;
}

inline bool verify(float got, double ref) {
  const bool ok = std::fabs(double(got) - ref) <= 1e-4 * ref;
  if (!ok) std::fprintf(stderr, "FEHLER: Summe %f statt %f\n", got, ref);
  return ok;
}

inline double bytes(std::size_t n) { return double(n) * sizeof(float); }

}  // namespace reduktion
}  // namespace bench
