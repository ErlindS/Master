#!/usr/bin/env python3
"""Fehlersteuerung für den eShop-Versuchsaufbau (Projektarbeit, Fehler F1–F7).

Beispiele:
    ./fault.py status                 # Zustand von Toxiproxy, Flags, angehaltenen Prozessen
    ./fault.py inject F1              # Fehler gezielt einschalten
    ./fault.py clear                  # alle Fehler zurücknehmen
    ./fault.py random --stufe S2      # zufälligen Fehler einschalten, Notiz verdeckt ablegen
    ./fault.py reveal <versuch-id>    # verdeckte Notiz nach der Diagnose aufdecken

Nur Python-Standardbibliothek. Läuft gegen `aspire run` (Dienste als Prozesse) und
gegen eine spätere Container-Variante (Docker Compose): Gibt es einen Container mit
dem Dienstnamen, wird `docker stop`/`docker pause` genutzt.
"""

from __future__ import annotations

import argparse
import json
import os
import random
import signal
import subprocess
import sys
import time
import urllib.error
import urllib.request
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
STATE_DIR = HERE / "state"
FAULTS_FILE = STATE_DIR / "faults.json"          # von allen Diensten live geladen (F4, F5, F7)
PAUSED_FILE = STATE_DIR / "paused.json"          # PIDs, die per SIGSTOP angehalten wurden (F3)
NOTES_DIR = STATE_DIR / "ground-truth"           # verdeckte Notizen
APPHOST = HERE.parent / "src" / "eShop.AppHost" / "eShop.AppHost.csproj"
TOXIPROXY = os.environ.get("TOXIPROXY_URL", "http://localhost:8474")

# Feste Stärke je Fehler. "Stufe" im Versuchsdesign meint S0–S3, nicht die Fehlerstärke.
F1_LATENCY_MS = 300      # zusätzliche Antwortzeit je Datenbank-Antwort
F5_CPU_BURN_MS = 300     # CPU-Zeit je Bestellung in Ordering.API

FAULTS = {
    "F1": "Datenbank langsam (Toxiproxy latency auf postgres)",
    "F2": "Basket.API ausgefallen (stop)",
    "F3": "OrderProcessor hängt (pause / SIGSTOP)",
    "F4": "Fehler in Geschäftslogik verschluckt (Catalog.API Bestandsprüfung)",
    "F5": "Dienst rechnet zu lange (Ordering.API CreateOrder, CPU)",
    "F6": "Redis nicht erreichbar (Toxiproxy-Proxy redis deaktiviert)",
    "F7": "falsche Konfiguration (PaymentOptions:PaymentSucceeded=false)",
}


# ---------------------------------------------------------------- Hilfsfunktionen

def toxiproxy(method: str, path: str, body: dict | None = None):
    data = json.dumps(body).encode() if body is not None else None
    req = urllib.request.Request(TOXIPROXY + path, data=data, method=method,
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            raw = resp.read()
            return json.loads(raw) if raw else None
    except urllib.error.URLError as e:
        sys.exit(f"Toxiproxy nicht erreichbar unter {TOXIPROXY}: {e}")


def read_json(path: Path, default):
    try:
        return json.loads(path.read_text())
    except (FileNotFoundError, json.JSONDecodeError):
        return default


def write_flags(flags: dict):
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    # In-place schreiben (kein rename), damit der FileSystemWatcher der Dienste anschlägt.
    with open(FAULTS_FILE, "w") as f:
        json.dump(flags, f, indent=2)
        f.write("\n")


def set_flag(section: str, key: str, value):
    flags = read_json(FAULTS_FILE, {})
    flags.setdefault(section, {})[key] = value
    write_flags(flags)


def docker_container(service: str) -> str | None:
    """Name eines laufenden/pausierten Containers für den Dienst, sonst None."""
    try:
        out = subprocess.run(["docker", "ps", "-a", "--format", "{{.Names}}"],
                             capture_output=True, text=True, check=True).stdout.split()
    except (FileNotFoundError, subprocess.CalledProcessError):
        return None
    candidates = [n for n in out if n == service or n.startswith(service + "-") or n.endswith("-" + service + "-1")]
    return candidates[0] if candidates else None


def aspire_resource(resource: str, command: str):
    subprocess.run(["aspire", "resource", resource, command, "--apphost", str(APPHOST),
                    "--non-interactive", "--nologo"], check=True)


def project_pids(project: str) -> list[int]:
    """PIDs des laufenden .NET-Prozesses eines Projekts (aspire run startet die gebaute DLL/EXE)."""
    pids = []
    for proc in Path("/proc").iterdir():
        if not proc.name.isdigit():
            continue
        try:
            cmd = (proc / "cmdline").read_bytes().split(b"\0")
        except OSError:
            continue
        args = [a.decode(errors="ignore") for a in cmd if a]
        if not args:
            continue
        exe = Path(args[0]).name
        dll = next((Path(a).name for a in args[1:2] if a.endswith(".dll")), "")
        if exe == project or dll == project + ".dll":
            pids.append(int(proc.name))
    return pids


# ---------------------------------------------------------------- Fehler

def inject(fault: str):
    if fault == "F1":
        toxiproxy("POST", "/proxies/postgres/toxics", {
            "name": "f1_latency", "type": "latency", "stream": "downstream", "toxicity": 1.0,
            "attributes": {"latency": F1_LATENCY_MS, "jitter": 0}})
    elif fault == "F2":
        c = docker_container("basket-api")
        if c:
            subprocess.run(["docker", "stop", c], check=True, capture_output=True)
        else:
            aspire_resource("basket-api", "stop")
    elif fault == "F3":
        c = docker_container("order-processor")
        if c:
            subprocess.run(["docker", "pause", c], check=True, capture_output=True)
        else:
            pids = project_pids("OrderProcessor")
            if not pids:
                sys.exit("OrderProcessor-Prozess nicht gefunden.")
            for pid in pids:
                os.kill(pid, signal.SIGSTOP)
            STATE_DIR.mkdir(parents=True, exist_ok=True)
            PAUSED_FILE.write_text(json.dumps(pids))
    elif fault == "F4":
        set_flag("Faults", "SwallowStockValidation", True)
    elif fault == "F5":
        set_flag("Faults", "CpuBurnMs", F5_CPU_BURN_MS)
    elif fault == "F6":
        toxiproxy("POST", "/proxies/redis", {"enabled": False})
    elif fault == "F7":
        set_flag("PaymentOptions", "PaymentSucceeded", False)
    else:
        sys.exit(f"Unbekannter Fehler {fault}. Erlaubt: {', '.join(FAULTS)}")


def clear():
    # F1, F6: alle Toxics entfernen, alle Proxies wieder aktivieren
    toxiproxy("POST", "/reset")
    # F4, F5, F7
    write_flags({})
    # F3
    for pid in read_json(PAUSED_FILE, []):
        try:
            os.kill(pid, signal.SIGCONT)
        except ProcessLookupError:
            pass
    PAUSED_FILE.unlink(missing_ok=True)
    c = docker_container("order-processor")
    if c:
        subprocess.run(["docker", "unpause", c], capture_output=True)
    # F2
    c = docker_container("basket-api")
    if c:
        subprocess.run(["docker", "start", c], capture_output=True)
    elif not project_pids("Basket.API"):
        aspire_resource("basket-api", "start")


def status():
    proxies = toxiproxy("GET", "/proxies")
    for name, p in sorted(proxies.items()):
        toxics = ", ".join(f"{t['name']}({t['type']})" for t in p["toxics"]) or "keine"
        print(f"toxiproxy {name:9} enabled={p['enabled']!s:5} {p['listen']} -> {p['upstream']}  toxics: {toxics}")
    print(f"faults.json: {json.dumps(read_json(FAULTS_FILE, {}))}")
    print(f"Basket.API PIDs: {project_pids('Basket.API') or '-'}")
    paused = read_json(PAUSED_FILE, [])
    print(f"OrderProcessor PIDs: {project_pids('OrderProcessor') or '-'}  angehalten: {paused or '-'}")


def random_trial(stufe: str | None):
    fault = random.SystemRandom().choice(list(FAULTS))
    trial = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S") + f"-{random.SystemRandom().randrange(16**4):04x}"
    inject(fault)
    NOTES_DIR.mkdir(parents=True, exist_ok=True)
    (NOTES_DIR / f"{trial}.json").write_text(json.dumps({
        "versuch": trial,
        "fehler": fault,
        "beschreibung": FAULTS[fault],
        "stufe": stufe,
        "eingeschaltet_utc": datetime.now(timezone.utc).isoformat(),
        "eingeschaltet_unix": time.time(),
    }, indent=2, ensure_ascii=False))
    # Bewusst nur die Versuchs-ID ausgeben, nicht den Fehler.
    print(trial)


def reveal(trial: str):
    path = NOTES_DIR / f"{trial}.json"
    if not path.exists():
        sys.exit(f"Keine Notiz für Versuch {trial}.")
    print(path.read_text())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("status")
    sub.add_parser("list")
    p = sub.add_parser("inject"); p.add_argument("fault", choices=list(FAULTS))
    sub.add_parser("clear")
    p = sub.add_parser("random"); p.add_argument("--stufe", choices=["S0", "S1", "S2", "S3"])
    p = sub.add_parser("reveal"); p.add_argument("trial")
    a = ap.parse_args()

    if a.cmd == "status":
        status()
    elif a.cmd == "list":
        for k, v in FAULTS.items():
            print(f"{k}  {v}")
    elif a.cmd == "inject":
        inject(a.fault)
        print(f"{a.fault} aktiv: {FAULTS[a.fault]}")
    elif a.cmd == "clear":
        clear()
        print("Alle Fehler zurückgenommen.")
    elif a.cmd == "random":
        random_trial(a.stufe)
    elif a.cmd == "reveal":
        reveal(a.trial)


if __name__ == "__main__":
    main()
