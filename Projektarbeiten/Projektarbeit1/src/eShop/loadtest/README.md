# Lasttests (k6)

Voraussetzung: Die Aspire-AppHost läuft. Alle Befehle werden im eShop-Wurzelordner ausgeführt.

| Skript | Zweck |
|---|---|
| `eshop.js` | Messlauf: feste Ankunftsrate, 80 % Stöbern / 20 % Bestellen, Phasen Aufwärmen / Messung / Ende |
| `catalog.js` | Einfacher Test nur gegen Katalog und WebApp (geschlossenes Modell mit VUs) |

```bash
k6 run -e PROFILE=smoke loadtest/eshop.js          # 30 s Funktionstest, 2 Sessions/s
k6 run loadtest/eshop.js                           # Messlauf: 2 min + 5 min + 1 min, 10 Sessions/s
k6 run -e RATE=25 -e RUN_ID=S1-25-01 loadtest/eshop.js
```

## Ablauf einer Session

- **Stöbern (browse):** WebApp-Startseite, Artikelliste (zufällige Seite), Filter nach Typ und Marke, 1–3 Artikel mit Bild und Detailseite.
- **Bestellen (order):** Login als `alice` oder `bob`, 1–3 Artikel ansehen, Warenkorb per gRPC (`UpdateBasket`, `GetBasket`), Kartentypen abrufen, Bestellung anlegen (`POST /api/orders`), Warenkorb leeren, Bestellliste abrufen.
  Danach läuft die asynchrone Verarbeitung über RabbitMQ (OrderProcessor, PaymentProcessor, Catalog-Lagerbestand).

Der Mix ist deterministisch: Bei `ORDER_SHARE=0.2` ist genau jede 5. Session eine Bestellung.

## Parameter (`-e NAME=wert`)

| Name | Standard | Bedeutung |
|---|---|---|
| `PROFILE` | `messlauf` | `smoke` für einen kurzen Funktionstest |
| `RATE` | 10 (smoke: 2) | neue Sessions pro Sekunde (offenes Modell) |
| `WARMUP` / `DURATION` / `COOLDOWN` | `2m` / `5m` / `1m` | Dauer der Phasen; in der Ende-Phase wird keine Last erzeugt |
| `ORDER_SHARE` | `0.2` | Anteil der Bestell-Sessions |
| `THINK_TIME` | `0.5` | mittlere Denkzeit zwischen Seitenaufrufen in s (±50 %) |
| `INCLUDE_WEBAPP` | `true` | WebApp-Seiten mit aufrufen |
| `MAX_VUS` | max(50, 20×RATE) | Obergrenze der virtuellen User |
| `RUN_ID` | Zeitstempel | Kennung des Laufs: als Tag `run_id` an allen Metriken und im Dateinamen |
| `RESULTS_DIR` | `loadtest/results` | Ablage der Zusammenfassung (`<RUN_ID>-summary.json`) |
| `CATALOG_URL`, `WEBAPP_URL`, `ORDERING_URL`, `IDENTITY_URL`, `BASKET_GRPC` | Ports aus den `launchSettings.json` | Endpunkte |

## Gültigkeit eines Laufs

Die Schwellwerte gelten nur für die Messphase. Ist einer verletzt, endet k6 mit Exit-Code 99:

- Fehlerquote HTTP ≤ 1 %, Checks > 99 %, Bestellungen > 99 % erfolgreich
- `dropped_iterations == 0`: Die Zielrate wurde erreicht, k6 war nicht ausgelastet
- p95 < 2 s

Am Ende gibt k6 den tatsächlichen Mix und die erreichte Rate aus.

## Hinweise

- Bestellungen senken den Lagerbestand im Katalog (Start: 100 pro Artikel). Nach vielen Läufen werden Bestellungen wegen fehlenden Bestands abgelehnt (`OrderStockRejected`). Vor einer Messreihe deshalb die Datenbank zurücksetzen.
- Es gibt nur zwei Testnutzer. Alle Bestellungen verteilen sich also auf zwei Käufer.
- `setup()` legt für jeden Nutzer vorab eine Bestellung an. Sonst kollidieren gleichzeitige erste Bestellungen am Unique-Index `IX_buyers_IdentityGuid` (Race Condition im eShop).
