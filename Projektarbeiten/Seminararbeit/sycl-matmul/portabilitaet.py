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

Zeilen mit tile = "auto" stammen aus Stufe 5 (Kachelgröße zur Laufzeit
gewählt) und werden mit der besten festen Kachelgröße verglichen.

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
    gewaehlt = defaultdict(dict)
    with open(path, newline="") as f:
        for row in csv.DictReader(f, delimiter=";"):
            n, dev = int(row["n"]), row["auswahl"]
            # tile = Kachelgröße aus Stufe 3 oder "auto" = Stufe 5 (zur Laufzeit gewählt)
            tile = row["tile"] if row["tile"] == "auto" else int(row["tile"])
            if tile == "auto":
                gewaehlt[n][dev] = row.get("gewaehlt") or "?"
            if row["geraet"]:
                namen[dev] = row["geraet"]
            gf = float(row["gflops"]) if row["status"] == "ok" else None
            daten[n][dev][tile] = gf

    for n in sorted(daten):
        geraete = sorted(daten[n])
        tiles = sorted({t for d in geraete for t in daten[n][d] if t != "auto"})
        mit_auto = any("auto" in daten[n][d] for d in geraete)

        print(f"\n=== n = {n} ===")
        for d in geraete:
            print(f"  {d:12s} = {namen.get(d, '?')}")

        # 1. + 2. GFLOPS und Effizienz je Gerät
        # beste feste Kachelgröße je Gerät (Stufe 4) als Bezug
        beste = {d: max((g for t, g in daten[n][d].items() if g and t != "auto"),
                        default=0.0)
                 for d in geraete}
        kopf = "  T   " + "".join(f"{d:>26s}" for d in geraete) + f"{'PP':>8s}"
        print("\n" + kopf)
        print("  " + "-" * (len(kopf) - 2))

        pp_werte = {}
        for t in tiles + (["auto"] if mit_auto else []):
            zellen, effs = [], []
            for d in geraete:
                g = daten[n][d].get(t)
                if g is None:
                    zellen.append(f"{'läuft nicht':>26s}")
                    effs.append(0.0)
                else:
                    e = g / beste[d] if beste[d] else 0.0
                    zellen.append(f"{g:10.1f} GFLOPS ({e:5.0%})")
                    # Messrauschen kann Stufe 5 knapp über das beste feste T heben
                    effs.append(min(e, 1.0))
            pp = harmonic_mean(effs)
            pp_werte[t] = pp
            print(f"  {str(t):<4s}" + "".join(zellen) + f"{pp:8.2f}")

        # 3. Fazit
        print()
        for d in geraete:
            t_best = max((t for t in tiles if daten[n][d].get(t)),
                         key=lambda t: daten[n][d][t], default=None)
            print(f"  beste Kachel auf {d:12s}: T = {t_best}")
        fest = {t: p for t, p in pp_werte.items() if t != "auto"}
        t_pp = max(fest, key=fest.get)
        print(f"  bester fester Wert für alle Geräte: T = {t_pp} (PP = {fest[t_pp]:.2f})")
        if mit_auto:
            wahl = ", ".join(f"{d}: T = {gewaehlt[n].get(d, '?')}" for d in geraete)
            print(f"  Stufe 5 (zur Laufzeit gewählt):     PP = {pp_werte['auto']:.2f} ({wahl})")
        print("  pro Gerät von Hand abgestimmt:      PP = 1.00 (per Definition)")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "kacheln.csv")
