# Fehlerinjektion F1–F7

Fehlersteuerung für den Versuchsaufbau der Projektarbeit (siehe `Ausarbeitung/Versuchsdesign.tex`, Abschnitt „Fehler“).

```bash
aspire run --apphost src/eShop.AppHost/eShop.AppHost.csproj   # eShop inkl. Toxiproxy starten
./fault-injection/fault.py status          # Zustand anzeigen
./fault-injection/fault.py inject F1       # Fehler gezielt einschalten
./fault-injection/fault.py clear           # alle Fehler zurücknehmen
./fault-injection/fault.py random --stufe S2   # Zufallsfehler, gibt nur die Versuchs-ID aus
./fault-injection/fault.py reveal <id>     # verdeckte Notiz aufdecken (state/ground-truth/)
```

| ID | Fehler | Umsetzung | Symptom |
|----|--------|-----------|---------|
| F1 | Datenbank langsam | Toxiproxy `latency` 300 ms (downstream) auf Proxy `postgres` | alle DB-Zugriffe (Catalog, Ordering, OrderProcessor, Identity, Webhooks) langsamer |
| F2 | Basket.API ausgefallen | `aspire resource basket-api stop` (bzw. `docker stop`) | Warenkorb/Bestellung schlägt fehl |
| F3 | OrderProcessor hängt | `SIGSTOP` auf den Prozess (bzw. `docker pause`) | Bestellungen bleiben in „Submitted“ |
| F4 | Fehler in Geschäftslogik verschluckt | Flag `Faults:SwallowStockValidation` in Catalog.API (Bestandsprüfung) | Bestellungen bleiben in „AwaitingValidation“, kein Fehler im Log (nur Debug) |
| F5 | Dienst rechnet zu lange | Flag `Faults:CpuBurnMs` = 300 in Ordering.API `CreateOrderCommandHandler` (echte CPU-Last) | Bestellen langsam, CPU von ordering-api hoch |
| F6 | Redis nicht erreichbar | Toxiproxy-Proxy `redis` deaktiviert (Verbindungen werden geschlossen/abgewiesen) | Basket.API-Fehler |
| F7 | falsche Konfiguration | `PaymentOptions:PaymentSucceeded=false` für PaymentProcessor | alle Zahlungen scheitern, Bestellungen werden storniert |

## Aufbau

- **Toxiproxy** (`ghcr.io/shopify/toxiproxy:2.12.0`) wird vom AppHost gestartet (`src/eShop.AppHost/FaultInjection.cs`).
  Proxies: `toxiproxy.json`. API fest auf `http://localhost:8474`.
  Die Dienste erhalten Connection-Strings, die auf Toxiproxy zeigen (nur der Port wird ersetzt; Passwort/TLS bleiben).
  Toxiproxy läuft in **jedem** Lauf mit, damit der Aufbau für FF1/FF2/FF4 und FF3 identisch ist.
- **Flags (F4, F5, F7)**: Alle Dienste laden `state/faults.json` mit `reloadOnChange` (in `eShop.ServiceDefaults`).
  Die Datei überschreibt appsettings und Umgebungsvariablen und wird beim Start des AppHost auf `{}` zurückgesetzt.
  Dadurch lassen sich die Fehler zur Laufzeit ohne Neustart schalten. F7 kann alternativ beim Start als
  Umgebungsvariable `PaymentOptions__PaymentSucceeded=false` gesetzt werden (dann mit Neustart).
- **Fehlerstärke** ist je Fehler fest (`F1_LATENCY_MS`, `F5_CPU_BURN_MS` in `fault.py`).
  „Stufe“ im Versuchsdesign meint die Instrumentierungsstufe S0–S3; `--stufe` wird nur in der Notiz vermerkt.
- `state/` ist nicht versioniert (Laufzeitzustand, verdeckte Notizen).
