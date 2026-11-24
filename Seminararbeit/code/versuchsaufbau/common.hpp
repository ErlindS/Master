// common.hpp – gemeinsame Hilfsfunktionen für alle Messprogramme
//
// Wird von den SYCL-, CUDA- und CPU-Programmen gleichermaßen benutzt, damit
// Initialisierung, Ergebnisprüfung und Ausgabeformat überall identisch sind.
//
// Ausgabe: eine Zeile pro Messung (Rohdaten, nicht schon gemittelt), damit
// die Auswertung Median und Konfidenzintervalle selbst bilden kann:
//
//   impl;auswahl;klasse;geraet;kernel;variante;n;lauf;wdh;zeit_ms;leistung;einheit;status
//
//   impl      dpcpp | acpp | cuda | openblas | openmp
//   auswahl   Geräteauswahl beim Start (Umgebungsvariable AUSWAHL), z. B. cuda:gpu
//   klasse    cpu | gpu
//   kernel    gemm | triad | transfer
//   lauf      Nummer des unabhängigen Programmstarts (Umgebungsvariable LAUF)
//   wdh       0 = erster Lauf (inkl. JIT/Initialisierung), 1..R = Messläufe
//   status    ok | erster_lauf | tuning | startet_nicht | falsch
#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace va {

struct Args {
  std::string variante;
  std::size_t n = 1024;
  int reps = 20;
};

// Aufruf:  ./programm <variante> <n> [wiederholungen]
inline Args parse_args(int argc, char** argv, const char* varianten) {
  Args a;
  if (argc < 3) {
    std::fprintf(stderr, "Aufruf: %s <%s> <n> [wiederholungen]\n", argv[0], varianten);
    std::exit(2);
  }
  a.variante = argv[1];
  a.n = std::strtoull(argv[2], nullptr, 10);
  if (argc > 3) a.reps = std::atoi(argv[3]);
  if (a.n == 0 || a.reps < 1) {
    std::fprintf(stderr, "ungültige Argumente\n");
    std::exit(2);
  }
  return a;
}

inline std::string env_or(const char* name, const char* fallback) {
  const char* v = std::getenv(name);
  return v ? v : fallback;
}

// Semikolons im Gerätenamen würden die CSV zerstören.
inline std::string csv_safe(std::string s) {
  std::replace(s.begin(), s.end(), ';', ',');
  return s;
}

// Reproduzierbare Zufallswerte im Bereich [-1, 1].
inline void fill_random(float* m, std::size_t count, unsigned seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  for (std::size_t i = 0; i < count; ++i) m[i] = dist(gen);
}

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

// 64 zufällige Einträge von C = A·B in double nachrechnen.
inline bool verify_gemm(const float* A, const float* B, const float* C, std::size_t n) {
  std::mt19937 gen(42);
  std::uniform_int_distribution<std::size_t> idx(0, n - 1);
  for (int s = 0; s < 64; ++s) {
    const std::size_t i = idx(gen), j = idx(gen);
    double ref = 0.0;
    for (std::size_t k = 0; k < n; ++k) ref += double(A[i * n + k]) * double(B[k * n + j]);
    const double tol = 1e-3 * std::max(1.0, std::sqrt(double(n)));
    if (std::fabs(double(C[i * n + j]) - ref) > tol) {
      std::fprintf(stderr, "FEHLER bei C[%zu][%zu]: %f statt %f\n", i, j, C[i * n + j], ref);
      return false;
    }
  }
  return true;
}

// Triade a = b + s·c an 64 zufälligen Stellen prüfen.
inline bool verify_triad(const float* a, const float* b, const float* c, float s,
                         std::size_t n) {
  std::mt19937 gen(42);
  std::uniform_int_distribution<std::size_t> idx(0, n - 1);
  for (int k = 0; k < 64; ++k) {
    const std::size_t i = idx(gen);
    if (std::fabs(a[i] - (b[i] + s * c[i])) > 1e-5f) return false;
  }
  return true;
}

constexpr float kTriadScalar = 3.0f;

inline double gemm_gflops(std::size_t n, double ms) {
  return 2.0 * double(n) * double(n) * double(n) / (ms * 1e6);
}

// Triade bewegt drei Vektoren (zwei lesen, einen schreiben).
inline double triad_gbs(std::size_t n, double ms) {
  return 3.0 * double(n) * sizeof(float) / (ms * 1e6);
}

// Name der CPU aus /proc/cpuinfo (für die Programme ohne SYCL-Gerätenamen).
inline std::string cpu_name() {
  std::ifstream f("/proc/cpuinfo");
  std::string line;
  while (std::getline(f, line))
    if (line.rfind("model name", 0) == 0) {
      const auto p = line.find(':');
      if (p != std::string::npos) return line.substr(p + 2);
    }
  return "CPU";
}

struct Kontext {
  std::string impl, klasse, geraet, kernel, variante;
  std::size_t n;
};

inline void zeile(const Kontext& k, int wdh, double ms, double leistung, const char* einheit,
                  const char* status) {
  static const std::string auswahl = env_or("AUSWAHL", "-");
  static const std::string lauf = env_or("LAUF", "0");
  std::printf("%s;%s;%s;%s;%s;%s;%zu;%s;%d;%.6f;%.3f;%s;%s\n", k.impl.c_str(),
              auswahl.c_str(), k.klasse.c_str(), csv_safe(k.geraet).c_str(),
              k.kernel.c_str(), k.variante.c_str(), k.n, lauf.c_str(), wdh, ms, leistung,
              einheit, status);
  std::fflush(stdout);
}

}  // namespace va
