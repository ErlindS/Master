#!/usr/bin/env python3
"""Umstellungsaufwand messbar machen (Codezeilen statt Eindrücke).

    python3 aufwand.py [pfad/zu/sycl-matmul]   -> ergebnisse/aufwand.csv

1. Geänderte Codezeilen zwischen den Stufen 0 -> 1 -> 2 -> 3 des
   Beispielprojekts (Kommentare und Leerzeilen zählen nicht).
2. Länge des Kachel-Kernels in SYCL und CUDA (Abschnitt zwischen den
   Markierungen KERNEL-BEGIN und KERNEL-END), um den Schreibaufwand beider
   Modelle für denselben Algorithmus zu vergleichen.
"""

import csv
import difflib
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
STUFEN = ["stufe0_sequentiell.cpp", "stufe1_usm_shared.cpp",
          "stufe2_usm_device.cpp", "stufe3_tiling.cpp"]


def codezeilen(text):
    """Quelltext ohne Kommentare, Leerzeilen und Einrückung."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    zeilen = []
    for z in text.splitlines():
        z = re.sub(r"//.*", "", z).strip()
        if z:
            zeilen.append(z)
    return zeilen


def diff(a, b):
    hinzu = weg = 0
    for z in difflib.unified_diff(a, b, lineterm="", n=0):
        if z.startswith("+") and not z.startswith("+++"):
            hinzu += 1
        elif z.startswith("-") and not z.startswith("---"):
            weg += 1
    return hinzu, weg


def kernel_abschnitt(pfad):
    text = pfad.read_text()
    m = re.search(r"KERNEL-BEGIN.*?\n(.*?)//\s*KERNEL-END", text, flags=re.S)
    return codezeilen(m.group(1)) if m else []


def main(projekt):
    zeilen = []
    texte = [codezeilen((projekt / s).read_text()) for s in STUFEN]
    for i in range(1, len(STUFEN)):
        hinzu, weg = diff(texte[i - 1], texte[i])
        zeilen.append({"vergleich": f"Stufe {i - 1} -> {i}", "zeilen_vorher": len(texte[i - 1]),
                       "zeilen_nachher": len(texte[i]), "hinzugefuegt": hinzu, "entfernt": weg})

    for name, datei in (("Kachel-Kernel SYCL", "sycl_gemm.cpp"), ("Kachel-Kernel CUDA", "cuda_gemm.cu")):
        k = kernel_abschnitt(HIER / datei)
        zeilen.append({"vergleich": name, "zeilen_nachher": len(k)})

    ziel = HIER / "ergebnisse"
    ziel.mkdir(exist_ok=True)
    spalten = ["vergleich", "zeilen_vorher", "zeilen_nachher", "hinzugefuegt", "entfernt"]
    with open(ziel / "aufwand.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=spalten, delimiter=";")
        w.writeheader()
        w.writerows(zeilen)

    print(f"{'Vergleich':22s} {'vorher':>7s} {'nachher':>8s} {'+':>5s} {'-':>5s}")
    for z in zeilen:
        print(f"{z['vergleich']:22s} {z.get('zeilen_vorher', ''):>7} {z['zeilen_nachher']:>8} "
              f"{z.get('hinzugefuegt', ''):>5} {z.get('entfernt', ''):>5}")
    print(f"\n-> {ziel / 'aufwand.csv'}")


if __name__ == "__main__":
    main(Path(sys.argv[1]) if len(sys.argv) > 1 else HIER.parent / "sycl-matmul")
