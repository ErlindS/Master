#!/usr/bin/env python3
"""Auswertung des Kachelgrößen-Experiments (run_kacheln.sh).

Zeigt für jede Matrixgröße n:
  1. GFLOPS je Gerät und Kachelgröße
  2. Effizienz je Gerät: GFLOPS / beste gemessene GFLOPS auf diesem Gerät
  3. Performance-Portabilität PP nach Pennycook et al. (2016):
     harmonischer Mittelwert der Effizienzen über alle Geräte,
     PP = 0, sobald der Code auf einem Gerät nicht läuft.

Die Effizienz bezieht sich hier auf die beste Kachelgröße desselben
Programms ("application efficiency"), nicht auf die Spitzenleistung der
Hardware.

    python3 portabilitaet.py kacheln.csv
"""

import csv
import sys
from collections import defaultdict


def harmonic_mean(values):
    if not values or any(v == 0 for v in values):
        return 0.0
    return len(values) / sum(1.0 / v for v in values)


def main(path):
    # daten[n][geraet][tile] = gflops (None = läuft nicht / falsch)
    daten = defaultdict(lambda: defaultdict(dict))
    namen = {}
    with open(path, newline="") as f:
        for row in csv.DictReader(f, delimiter=";"):
            n, dev, tile = int(row["n"]), row["auswahl"], int(row["tile"])
            if row["geraet"]:
                namen[dev] = row["geraet"]
            gf = float(row["gflops"]) if row["status"] == "ok" else None
            daten[n][dev][tile] = gf

    for n in sorted(daten):
        geraete = sorted(daten[n])
        tiles = sorted({t for d in geraete for t in daten[n][d]})

        print(f"\n=== n = {n} ===")
        for d in geraete:
            print(f"  {d:12s} = {namen.get(d, '?')}")

        # 1. + 2. GFLOPS und Effizienz je Gerät
        beste = {d: max((g for g in daten[n][d].values() if g), default=0.0)
                 for d in geraete}
        kopf = "  T   " + "".join(f"{d:>26s}" for d in geraete) + f"{'PP':>8s}"
        print("\n" + kopf)
        print("  " + "-" * (len(kopf) - 2))

        pp_werte = {}
        for t in tiles:
            zellen, effs = [], []
            for d in geraete:
                g = daten[n][d].get(t)
                if g is None:
                    zellen.append(f"{'läuft nicht':>26s}")
                    effs.append(0.0)
                else:
                    e = g / beste[d]
                    zellen.append(f"{g:10.1f} GFLOPS ({e:5.0%})")
                    effs.append(e)
            pp = harmonic_mean(effs)
            pp_werte[t] = pp
            print(f"  {t:<4d}" + "".join(zellen) + f"{pp:8.2f}")

        # 3. Fazit
        print()
        for d in geraete:
            t_best = max((t for t in tiles if daten[n][d].get(t)),
                         key=lambda t: daten[n][d][t], default=None)
            print(f"  beste Kachel auf {d:12s}: T = {t_best}")
        t_pp = max(pp_werte, key=pp_werte.get)
        print(f"  bester fester Wert für alle Geräte: T = {t_pp} (PP = {pp_werte[t_pp]:.2f})")
        print("  pro Gerät abgestimmt:                        PP = 1.00 (per Definition)")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "kacheln.csv")
