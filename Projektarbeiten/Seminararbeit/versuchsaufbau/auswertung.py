#!/usr/bin/env python3
"""Auswertung der Rohdaten aus run_all.sh.

    python3 auswertung.py messdaten/roh_<datum>.csv [ausgabeordner]

Schritte:
  1. Median und 95-%-Konfidenzintervall je Konfiguration. Das Intervall wird
     per zweistufigem Bootstrap bestimmt (erst Läufe, dann Wiederholungen
     ziehen), damit Streuung zwischen Programmstarts mit eingeht.
  2. Effizienz jeder Variante
       - relativ zur Bibliothek (cuBLAS auf der GPU, OpenBLAS auf der CPU)
       - relativ zur besten eigenen SYCL-Variante (wie bisher in der Arbeit)
  3. Performance-Portabilität nach Pennycook et al. je SYCL-Variante, für
     beide Effizienzbegriffe, inklusive "beste Kachel je Gerät" und "auto".
  4. SYCL gegen CUDA bei identischem Algorithmus, DPC++ gegen AdaptiveCpp.
  5. Prüfung der Hypothesen H1–H5 (Schwellen siehe unten, VOR der Messung
     festlegen!) und Datendateien für pgfplots.

Nur Standardbibliothek, keine weiteren Pakete nötig.
"""

import csv
import math
import random
import statistics
import sys
from collections import defaultdict
from pathlib import Path

# --- Hypothesen-Schwellen (vor der Messung festlegen und nicht nachträglich ändern)
H1_MIN_ANTEIL = 0.70  # naiv erreicht auf der GPU >= 70 % der besten eigenen Variante
H2_MAX_NACHTEIL = 0.10  # SYCL höchstens 10 % langsamer als CUDA (gleicher Algorithmus)
H4_MIN_UNTERSCHIED = 0.10  # DPC++ und AdaptiveCpp unterscheiden sich um > 10 %
H5_MIN_EFFIZIENZ = 0.90  # SYCL-Triade erreicht >= 90 % der nativen Bandbreite

REFERENZ = {  # (kernel, klasse) -> (impl, variante) der Baseline
    ("gemm", "gpu"): ("cuda", "cublas"),
    ("gemm", "cpu"): ("openblas", "sgemm"),
    ("triad", "gpu"): ("cuda", "triad"),
    ("triad", "cpu"): ("openmp", "triad"),
}
SYCL_IMPLS = ("dpcpp", "acpp")
GEMM_VARIANTEN = ("naiv", "tile8", "tile16", "tile32", "tile64", "auto")
TILES = ("tile8", "tile16", "tile32", "tile64")
BOOTSTRAP = 2000


# --------------------------------------------------------------------------- Daten
class Daten:
    def __init__(self, pfad):
        self.mess = defaultdict(lambda: defaultdict(list))  # key -> lauf -> [ms]
        self.erster = defaultdict(list)
        self.tuning_ms = defaultdict(list)
        self.gewaehlt = defaultdict(list)  # auto: gewählte Kachelgrößen
        self.ausfall = set()
        self.falsch = set()
        self.geraet = {}  # (impl, klasse) -> Name
        with open(pfad, newline="") as f:
            for r in csv.DictReader(f, delimiter=";"):
                var = r["variante"]
                if var.startswith("auto_T"):  # gewählte Kachel steht in der Tuning-Zeile
                    var = "auto"
                key = (r["impl"], r["klasse"], r["kernel"], var, int(r["n"]))
                self.geraet[(r["impl"], r["klasse"])] = r["geraet"]
                st = r["status"]
                if st == "ok":
                    self.mess[key][r["lauf"]].append(float(r["zeit_ms"]))
                elif st == "erster_lauf":
                    self.erster[key].append(float(r["zeit_ms"]))
                elif st == "tuning":
                    self.tuning_ms[key].append(float(r["zeit_ms"]))
                    self.gewaehlt[key].append(int(float(r["leistung"])))
                elif st == "startet_nicht":
                    self.ausfall.add(key)
                elif st == "falsch":
                    self.falsch.add(key)


def leistung(kernel, n, ms):
    if kernel == "gemm":
        return 2.0 * n**3 / (ms * 1e6)  # GFLOPS
    if kernel == "triad":
        return 12.0 * n / (ms * 1e6)  # GB/s
    return 12.0 * n * n / (ms * 1e6)  # transfer: 3 Matrizen à 4n² Byte, GB/s


def einheit(kernel):
    return "GFLOPS" if kernel == "gemm" else "GB/s"


def median_ci(laeufe, rng):
    """Median aller Werte + 95-%-Bootstrap-Intervall (Läufe, dann Wiederholungen)."""
    gruppen = [v for v in laeufe.values() if v]
    alle = [x for g in gruppen for x in g]
    med = statistics.median(alle)
    if len(alle) < 2:
        return med, med, med
    meds = []
    for _ in range(BOOTSTRAP):
        stich = []
        for g in rng.choices(gruppen, k=len(gruppen)):
            stich.extend(rng.choices(g, k=len(g)))
        meds.append(statistics.median(stich))
    meds.sort()
    return med, meds[int(0.025 * BOOTSTRAP)], meds[int(0.975 * BOOTSTRAP) - 1]


def harmonisch(werte):
    if not werte or any(w <= 0 for w in werte):
        return 0.0
    return len(werte) / sum(1.0 / w for w in werte)


# --------------------------------------------------------------------------- Auswertung
def auswerten(pfad, ausgabe):
    d = Daten(pfad)
    rng = random.Random(1)
    ausgabe.mkdir(parents=True, exist_ok=True)
    (ausgabe / "plots").mkdir(exist_ok=True)

    # 1. Statistik je Konfiguration ------------------------------------------
    stat = {}  # key -> dict
    for key in sorted(set(d.mess) | d.ausfall):
        impl, klasse, kernel, var, n = key
        e = {"impl": impl, "klasse": klasse, "geraet": d.geraet.get((impl, klasse), ""),
             "kernel": kernel, "variante": var, "n": n}
        if key in d.falsch:
            e["status"] = "falsch"
        elif key in d.mess and key not in d.ausfall:
            med, lo, hi = median_ci(d.mess[key], rng)
            e.update(status="ok", laeufe=len(d.mess[key]),
                     messungen=sum(len(v) for v in d.mess[key].values()),
                     median_ms=med, ci_unten_ms=lo, ci_oben_ms=hi,
                     leistung=leistung(kernel, n, med),
                     leistung_ci_unten=leistung(kernel, n, hi),
                     leistung_ci_oben=leistung(kernel, n, lo))
        else:
            e["status"] = "startet_nicht"
        if d.erster.get(key):
            e["erster_lauf_ms"] = statistics.median(d.erster[key])
        if d.gewaehlt.get(key):
            e["gewaehltes_T"] = "/".join(str(t) for t in sorted(set(d.gewaehlt[key])))
            e["tuning_ms"] = statistics.median(d.tuning_ms[key])
        stat[key] = e

    def perf(impl, klasse, kernel, var, n):
        e = stat.get((impl, klasse, kernel, var, n))
        return e["leistung"] if e and e["status"] == "ok" else 0.0

    def ref_perf(kernel, klasse, n):
        ref_impl, ref_var = REFERENZ[(kernel, klasse)]
        return perf(ref_impl, klasse, kernel, ref_var, n)

    spalten = ["impl", "klasse", "geraet", "kernel", "variante", "n", "status", "laeufe",
               "messungen", "median_ms", "ci_unten_ms", "ci_oben_ms", "leistung",
               "leistung_ci_unten", "leistung_ci_oben", "erster_lauf_ms", "gewaehltes_T",
               "tuning_ms"]
    schreibe_csv(ausgabe / "konfigurationen.csv", spalten, stat.values())

    # 2. Effizienzen ---------------------------------------------------------
    eff = []
    for (impl, klasse, kernel, var, n), e in stat.items():
        if kernel not in ("gemm", "triad") or (kernel, klasse) not in REFERENZ:
            continue
        p = e.get("leistung", 0.0) if e["status"] == "ok" else 0.0
        ref = ref_perf(kernel, klasse, n)
        zeile = {"impl": impl, "klasse": klasse, "kernel": kernel, "variante": var, "n": n,
                 "leistung": round(p, 2), "effizienz_bibliothek": quote(p, ref)}
        if impl in SYCL_IMPLS and kernel == "gemm":
            beste = max(perf(impl, klasse, "gemm", v, n) for v in GEMM_VARIANTEN)
            zeile["effizienz_eigen"] = quote(p, beste)
        eff.append(zeile)
    schreibe_csv(ausgabe / "effizienz.csv", ["impl", "klasse", "kernel", "variante", "n",
                 "leistung", "effizienz_bibliothek", "effizienz_eigen"], eff)

    # 3. Performance-Portabilität -------------------------------------------
    pp = []
    for impl in SYCL_IMPLS:
        klassen = sorted({k[1] for k in stat if k[0] == impl and k[2] == "gemm"})
        if not klassen:
            continue
        for n in sorted({k[4] for k in stat if k[0] == impl and k[2] == "gemm"}):
            def effs(waehle):
                eigen, bib = [], []
                for kl in klassen:
                    var = waehle(kl)
                    p = perf(impl, kl, "gemm", var, n) if var else 0.0
                    beste = max(perf(impl, kl, "gemm", v, n) for v in GEMM_VARIANTEN)
                    ref = ref_perf("gemm", kl, n)
                    eigen.append(quote(p, beste) or 0.0)
                    bib.append(quote(p, ref) or 0.0)
                return eigen, bib

            faelle = [(v, (lambda kl, v=v: v)) for v in GEMM_VARIANTEN]
            faelle.append(("bestes_T_je_geraet",
                           lambda kl: max(TILES, key=lambda t: perf(impl, kl, "gemm", t, n))))
            for name, waehle in faelle:
                eigen, bib = effs(waehle)
                pp.append({"impl": impl, "n": n, "variante": name, "plattformen": "+".join(klassen),
                           "pp_eigen": round(harmonisch(eigen), 3),
                           "pp_bibliothek": round(harmonisch(bib), 3),
                           **{f"eff_eigen_{kl}": round(x, 3) for kl, x in zip(klassen, eigen)},
                           **{f"eff_bib_{kl}": round(x, 3) for kl, x in zip(klassen, bib)}})
    pp_spalten = ["impl", "n", "variante", "plattformen", "pp_eigen", "pp_bibliothek",
                  "eff_eigen_cpu", "eff_eigen_gpu", "eff_bib_cpu", "eff_bib_gpu"]
    schreibe_csv(ausgabe / "performance_portabilitaet.csv", pp_spalten, pp)

    # 4. Vergleiche ------------------------------------------------------------
    vergleich = []
    for (impl, klasse, kernel, var, n), e in stat.items():
        if impl not in SYCL_IMPLS or e["status"] != "ok":
            continue
        if klasse == "gpu" and kernel == "gemm" and var in ("naiv",) + TILES:
            vergleich.append(verhaeltnis(f"{impl} vs cuda", e, stat.get(("cuda", "gpu", kernel, var, n))))
        if kernel == "triad":
            ref = REFERENZ[("triad", klasse)]
            vergleich.append(verhaeltnis(f"{impl} vs {ref[0]}", e, stat.get((ref[0], klasse, "triad", "triad", n))))
        if impl == "dpcpp":
            vergleich.append(verhaeltnis("dpcpp vs acpp", e, stat.get(("acpp", klasse, kernel, var, n))))
    vergleich = [v for v in vergleich if v]
    schreibe_csv(ausgabe / "vergleiche.csv", ["vergleich", "klasse", "kernel", "variante", "n",
                 "zeit_a_ms", "zeit_b_ms", "faktor_a_durch_b", "gesichert"], vergleich)

    # 5. pgfplots-Daten ----------------------------------------------------------
    for impl in SYCL_IMPLS:
        for n in sorted({z["n"] for z in pp if z["impl"] == impl}):
            zeilen = [z for z in pp if z["impl"] == impl and z["n"] == n]
            with open(ausgabe / "plots" / f"pp_{impl}_n{n}.dat", "w") as f:
                f.write("variante pp_eigen pp_bibliothek\n")
                for z in zeilen:
                    f.write(f"{z['variante']} {z['pp_eigen']} {z['pp_bibliothek']}\n")
    for (kernel, klasse) in REFERENZ:
        for n in sorted({z["n"] for z in eff if z["kernel"] == kernel and z["klasse"] == klasse}):
            zeilen = [z for z in eff if z["kernel"] == kernel and z["klasse"] == klasse and z["n"] == n]
            with open(ausgabe / "plots" / f"effizienz_{kernel}_{klasse}_n{n}.dat", "w") as f:
                f.write("konfiguration effizienz_bibliothek\n")
                for z in sorted(zeilen, key=lambda z: (z["impl"], z["variante"])):
                    f.write(f"{z['impl']}-{z['variante']} {z['effizienz_bibliothek'] or 0}\n")

    # 6. Hypothesen ------------------------------------------------------------
    text = hypothesen(stat, pp, vergleich, perf, ref_perf)
    (ausgabe / "zusammenfassung.md").write_text(text)
    print(text)
    print(f"\nTabellen und Plot-Daten in: {ausgabe}/")


def quote(a, b):
    return round(a / b, 3) if a and b else None


def verhaeltnis(name, a, b):
    if not b or b.get("status") != "ok":
        return None
    gesichert = a["ci_oben_ms"] < b["ci_unten_ms"] or b["ci_oben_ms"] < a["ci_unten_ms"]
    return {"vergleich": name, "klasse": a["klasse"], "kernel": a["kernel"],
            "variante": a["variante"], "n": a["n"], "zeit_a_ms": round(a["median_ms"], 4),
            "zeit_b_ms": round(b["median_ms"], 4),
            "faktor_a_durch_b": round(a["median_ms"] / b["median_ms"], 3),
            "gesichert": "ja" if gesichert else "nein (Intervalle überlappen)"}


def schreibe_csv(pfad, spalten, zeilen):
    with open(pfad, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=spalten, delimiter=";", extrasaction="ignore")
        w.writeheader()
        for z in zeilen:
            w.writerow({k: (f"{v:.4f}" if isinstance(v, float) else v) for k, v in z.items()})


def fmt(x, pct=True):
    if x is None:
        return "–"
    return f"{100 * x:.0f} %" if pct else f"{x:.2f}"


def hypothesen(stat, pp, vergleich, perf, ref_perf):
    out = ["# Zusammenfassung der Hypothesen", ""]
    gemm_n = sorted({k[4] for k in stat if k[2] == "gemm"})
    if not gemm_n:
        return "Keine GEMM-Messungen gefunden."
    n = gemm_n[-1]
    out.append(f"Ausgewertet für die größte Matrixgröße n = {n}.\n")

    # H1
    out.append(f"## H1 – Minimale Portierung (naiv) erreicht auf der GPU >= {fmt(H1_MIN_ANTEIL)} der besten eigenen Variante")
    for impl in SYCL_IMPLS:
        p = perf(impl, "gpu", "gemm", "naiv", n)
        beste = max(perf(impl, "gpu", "gemm", v, n) for v in GEMM_VARIANTEN)
        if p and beste:
            a = p / beste
            out.append(f"- {impl}: {fmt(a)} → {'bestätigt' if a >= H1_MIN_ANTEIL else 'widerlegt'}")
    out.append("")

    # H2
    out.append(f"## H2 – SYCL ist bei gleichem Algorithmus höchstens {fmt(H2_MAX_NACHTEIL)} langsamer als CUDA")
    for v in vergleich:
        if v["vergleich"].endswith("vs cuda") and v["n"] == n:
            f = v["faktor_a_durch_b"]
            ok = f <= 1 + H2_MAX_NACHTEIL
            out.append(f"- {v['vergleich']}, {v['variante']}: Zeit SYCL/CUDA = {f:.2f} "
                       f"(gesichert: {v['gesichert']}) → {'bestätigt' if ok else 'widerlegt'}")
    out.append("")

    # H3
    out.append("## H3 – Keine feste Kachelgröße ist überall optimal; gerätebewusste Auswahl ist portabler")
    for impl in SYCL_IMPLS:
        zeilen = {z["variante"]: z for z in pp if z["impl"] == impl and z["n"] == n}
        if not zeilen:
            continue
        beste_je = {kl: max(TILES, key=lambda t: perf(impl, kl, "gemm", t, n)) for kl in ("cpu", "gpu")
                    if any(perf(impl, kl, "gemm", t, n) for t in TILES)}
        fest = max((zeilen[t] for t in TILES if t in zeilen), key=lambda z: z["pp_eigen"])
        auto = zeilen.get("auto")
        out.append(f"- {impl}: beste Kachel je Gerät {beste_je}; bester fester Wert {fest['variante']} "
                   f"mit PP {fmt(fest['pp_eigen'], False)} (Bibliothek: {fmt(fest['pp_bibliothek'], False)})")
        if auto:
            gew = {kl: stat.get((impl, kl, 'gemm', 'auto', n), {}).get('gewaehltes_T') for kl in beste_je}
            out.append(f"  auto: PP {fmt(auto['pp_eigen'], False)} (Bibliothek: {fmt(auto['pp_bibliothek'], False)}), "
                       f"gewählte T {gew}")
        unterschiedlich = len(set(beste_je.values())) > 1
        out.append(f"  → Optimum geräteabhängig: {'ja' if unterschiedlich else 'nein'}")
    out.append("")

    # H4
    out.append(f"## H4 – DPC++ und AdaptiveCpp unterscheiden sich um mehr als {fmt(H4_MIN_UNTERSCHIED)}")
    treffer = [v for v in vergleich if v["vergleich"] == "dpcpp vs acpp" and v["n"] == n]
    if not treffer:
        out.append("- keine AdaptiveCpp-Messungen vorhanden")
    for v in treffer:
        f = v["faktor_a_durch_b"]
        out.append(f"- {v['klasse']} {v['kernel']} {v['variante']}: Zeit DPC++/AdaptiveCpp = {f:.2f} "
                   f"(gesichert: {v['gesichert']})")
    out.append("")

    # H5
    out.append(f"## H5 – Die speichergebundene Triade ist portabler: SYCL erreicht >= {fmt(H5_MIN_EFFIZIENZ)} der nativen Bandbreite")
    triad_n = sorted({k[4] for k in stat if k[2] == "triad"})
    if triad_n:
        tn = triad_n[-1]
        for impl in SYCL_IMPLS:
            for kl in ("cpu", "gpu"):
                p = perf(impl, kl, "triad", "triad", tn)
                ref = ref_perf("triad", kl, tn)
                if p and ref:
                    out.append(f"- {impl} {kl}: {p:.0f} GB/s = {fmt(p / ref)} der nativen Referenz "
                               f"→ {'bestätigt' if p / ref >= H5_MIN_EFFIZIENZ else 'widerlegt'}")
    out.append("")

    # Bibliotheks-Baseline
    out.append("## Abstand zur Bibliothek (Effizienz bestes SYCL / Bibliothek)")
    for impl in SYCL_IMPLS:
        for kl in ("cpu", "gpu"):
            beste = max(perf(impl, kl, "gemm", v, n) for v in GEMM_VARIANTEN)
            ref = ref_perf("gemm", kl, n)
            if beste and ref:
                out.append(f"- {impl} {kl}: {beste:.0f} GFLOPS = {fmt(beste / ref)} der Bibliothek ({ref:.0f} GFLOPS)")
    out.append("")

    # Anlaufkosten
    out.append("## Anlaufkosten (erster Lauf / Median), n = " + str(gemm_n[0]))
    for (impl, kl, kernel, var, nn), e in sorted(stat.items()):
        if nn == gemm_n[0] and kernel == "gemm" and var == "naiv" and e.get("erster_lauf_ms") and e["status"] == "ok":
            out.append(f"- {impl} {kl}: {e['erster_lauf_ms']:.2f} ms statt {e['median_ms']:.3f} ms "
                       f"(Faktor {e['erster_lauf_ms'] / e['median_ms']:.0f})")
    return "\n".join(out)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    quelle = Path(sys.argv[1])
    ziel = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("ergebnisse") / quelle.stem
    auswerten(quelle, ziel)
