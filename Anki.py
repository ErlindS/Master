import genanki

MODEL_ID = 1829384756
DECK_ID = 2938475610

style = """
.card {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    font-size: 19px;
    line-height: 1.5;
    color: #111111;
    background-color: #ffffff;
    padding: 25px;
    max-width: 700px;
    margin: 0 auto;
}
.nightMode .card {
    color: #eeeeee;
    background-color: #2c2c2c;
}
b { color: #0066cc; }
.nightMode b { color: #4da6ff; }
hr { border: 0; height: 1px; background: #cccccc; margin: 20px 0; }
.nightMode hr { background: #555555; }
ul, ol { padding-left: 25px; margin-top: 5px; }
li { margin-bottom: 5px; }
table {
    border-collapse: collapse;
    width: 100%;
    margin-top: 10px;
    margin-bottom: 10px;
}
th, td {
    border: 1px solid #cccccc;
    padding: 8px;
    text-align: left;
}
th {
    background-color: #f2f2f2;
}
.nightMode th {
    background-color: #3a3a3a;
    color: #eeeeee;
}
.nightMode td {
    border-color: #555555;
}
"""

model = genanki.Model(
    MODEL_ID,
    'SWA Clean Model',
    fields=[
        {'name': 'Question'},
        {'name': 'Answer'},
    ],
    templates=[
        {
            'name': 'Card 1',
            'qfmt': '<div class="card"><b>Frage:</b><br>{{Question}}</div>',
            'afmt': '<div class="card"><b>Frage:</b><br>{{Question}}<hr><b>Antwort:</b><br>{{Answer}}</div>',
        },
    ],
    css=style
)

anki_deck = genanki.Deck(
    DECK_ID,
    'Software-Architekturen :: Klausurwissen'
)

flashcards = [
    # ===== 1. Software-Engineering & Prozesse =====
    (
        "Was ist Software-Engineering? Nenne die Definitionen von Broy/Rombach und Sommerville.",
        "<b>Definitionen:</b>\n"
        "• <b>Broy/Rombach:</b> Technologie- und Management-Disziplin für die systematische Erstellung und Wartung großer Software-Systeme unter Verwendung bewährter Vorgehensweisen, Prinzipien, Methoden und Werkzeuge.\n"
        "• <b>Sommerville:</b> Ingenieure bauen Artefakte auf Basis von Theorien und Methoden – unter finanziellen und organisatorischen Constraints.\n\n"
        "<b>Kernaussage:</b> Systematik ist unabdingbar in Prozess, Analyse, Design und Test."
    ),
    (
        "Nenne die 4 Phasen des iterativen Vorgehens nach Larman. Was löst diese Phasen in modernen Umgebungen ab?",
        "<b>Die 4 Phasen (Larman):</b>\n"
        "1. <b>Evaluierung</b> (Phase I)\n"
        "2. <b>Planen</b>\n"
        "3. <b>Erstellung</b> (Phase II)\n"
        "4. <b>Inbetriebnahme</b>\n\n"
        "<b>Moderne Ablösung:</b>\n"
        "<b>CI/CD</b> (Continuous Integration/Continuous Delivery) ersetzt die Erstellungs- und Inbetriebnahme-Phase. Die Kern-Herausforderungen (Analyse, Design, Implementierung) bleiben jedoch identisch."
    ),
    (
        "Erkläre die Unterschiede zwischen CI, Continuous Delivery (CD) und Continuous Deployment (CD).",
        "<table>"
        "<tr><th>Konzept</th><th>Bedeutung</th></tr>"
        "<tr><td><b>CI (Continuous Integration)</b></td><td>Gemeinsame Code-Basis mit Feature-Branches. Integration per Pull-Request, abgesichert durch automatischen Build, Tests und Quality Gate.</td></tr>"
        "<tr><td><b>Continuous Delivery (CD)</b></td><td>Software ist <b>jederzeit auslieferbar</b>. Das eigentliche Deployment erfolgt nach manueller Freigabe.</td></tr>"
        "<tr><td><b>Continuous Deployment (CD)</b></td><td>Nach erfolgreichem Build und Test folgt das <b>vollautomatische Deployment</b> auf die Zielplattform (ohne manuelle Freigabe).</td></tr>"
        "</table>"
    ),
    (
        "Warum stellt ein Pull-Request im CI-Prozess ein Beispiel für \"Inversion of Control\" (IoC) dar?",
        "<b>Inversion of Control beim PR:</b>\n"
        "Entwickler können ihren Code <b>nicht selbstständig</b> in den Hauptzweig (Main-Branch) pushen.\n"
        "Stattdessen fordert der Entwickler an, dass sein Code <b>gezogen</b> wird. Die Integrationsplattform übernimmt die Kontrolle, prüft den Code automatisch (Build, Tests, Quality Gate) und erlaubt den Merge erst nach erfolgreicher Prüfung."
    ),
    (
        "Nenne typische Zielplattformen für das Deployment (CaaS, PaaS, SaaS, IaaS, App Stores) mit Beispielen.",
        "• <b>CaaS (Container as a Service):</b> Orchestrierung von Containern (z. B. Kubernetes, Amazon ECS).\n"
        "• <b>PaaS (Platform as a Service):</b> Cloud-Plattformen ohne Serververwaltung (z. B. Azure, GCP App Engine).\n"
        "• <b>SaaS-Hosting (Software as a Service):</b> Direktes Hosting von Frontends (z. B. Netlify, Vercel).\n"
        "• <b>IaaS (Infrastructure as a Service):</b> Virtuelle Maschinen (z. B. VMware, AWS EC2).\n"
        "• <b>App Stores:</b> Mobile Vertriebskanäle."
    ),
    (
        "Was unterscheidet Analyse und Design im Software-Engineering?",
        "• <b>Analyse:</b> Das Problem verstehen und beschreiben, sodass ein korrekter Entwurf möglich wird.\n"
        "→ Frage: <i>Welche Schnittstellen hat das System?</i>\n"
        "• <b>Design:</b> Die Lösung beschreiben, sodass die Anforderungen umgesetzt werden. Unterteilt in:\n"
        "  1. <i>Systementwurf:</i> Aufbrechen in Teilsysteme, Strategien für Steuerung/Datenhaltung.\n"
        "  2. <i>Objektentwurf:</i> Zuweisen von Verantwortlichkeiten zu Klassen/Objekten.\n"
        "→ Frage: <i>Wie wird die Aufgabe realisiert?</i>"
    ),
    (
        "Erkläre das Semiotische Dreieck (Ogden & Richards) im Kontext von Analyse & Design.",
        "<b>Das Semiotische Dreieck:</b>\n"
        "Erklärt die Relation zwischen:\n"
        "1. <b>Wort (Beschreibung):</b> Der Code, das Diagramm oder die Anforderung.\n"
        "2. <b>Begriff (Semantik):</b> Das Verständnis im Kopf des Entwicklers/Kunden.\n"
        "3. <b>Objekt (Realität):</b> Das tatsächlich gebaute Software-System.\n\n"
        "<b>Kernaussage für SE:</b>\n"
        "Stimmt das, was der Kunde will, mit dem überein, was der Entwickler glaubt bauen zu müssen und was am Ende gebaut wird? Konvergenz wird durch <b>Iterationen</b> erreicht."
    ),

    # ===== 2. Was ist Architektur & Sichten =====
    (
        "Wie lautet die Definition von Software-Architektur nach IEEE 1471-2000?",
        "<b>Definition:</b>\n"
        "\"The fundamental organization of a system embodied in its components, their relationships to each other, and to the environment, and the principles guiding its design and evolution.\"\n\n"
        "<b>Essentials:</b>\n"
        "Ein Bauplan für die Organisation des Gesamtsystems (Struktur, Schnittstellen, Dynamik, Prinzipien) von der Entwicklung bis zum Betrieb."
    ),
    (
        "Erkläre die Kernbegriffe nach IEEE 1471 und ihren Zusammenhang: System, Stakeholder, Concern, View, Viewpoint.",
        "• <b>System:</b> Sammlung von Komponenten zur Erfüllung bestimmter Funktionen.\n"
        "• <b>Stakeholder:</b> Person/Gruppe mit Interessen am System (Entwickler, Kunde, PM).\n"
        "• <b>Concern:</b> Ein Ziel, Wunsch oder Constraint (NFRs, Absichten).\n"
        "• <b>View:</b> Darstellung des Gesamtsystems aus Perspektive zusammenhängender Concerns.\n"
        "• <b>Viewpoint:</b> Schablone/Konvention zur Erstellung einer View.\n\n"
        "<b>Zusammenhang:</b> Ein System hat eine Architektur, dokumentiert in einer <i>Architectural Description (AD)</i>. Diese besteht aus <i>Views</i>, die gemäß <i>Viewpoints</i> erstellt wurden, um die <i>Concerns</i> der <i>Stakeholder</i> zu adressieren."
    ),
    (
        "Beschreibe die Sichten des 4+1 Sichtenmodells (Kruchten) und nenne typische Diagramme.",
        "<table>"
        "<tr><th>Sicht (View)</th><th>Fokus</th><th>Diagramme / Inhalt</th></tr>"
        "<tr><td><b>Logical</b></td><td>Anwendersicht (funktionale Bausteine)</td><td>Klassen-, Sequenz-, Komponentendiagramm</td></tr>"
        "<tr><td><b>Process</b></td><td>Nebenläufigkeit (Threads, Synchronisation)</td><td>Aktivitäts-, Sequenzdiagramm (NFR: Performance, Verfügbarkeit)</td></tr>"
        "<tr><td><b>Physical</b></td><td>Reale Struktur (SW-Verteilung auf HW)</td><td>Deployment-Diagramm (NFR: Skalierbarkeit, Ausfallsicherheit)</td></tr>"
        "<tr><td><b>Development</b></td><td>Entwicklersicht (Pakete, Bibliotheken)</td><td>Paket-, Klassen-, Deploymentdiagramm</td></tr>"
        "<tr><td><b>Use Case (+1)</b></td><td>Anforderungen (Szenarien)</td><td>Maßstab, an dem sich alle anderen Sichten orientieren</td></tr>"
        "</table>"
    ),
    (
        "Beschreibe das 3+1 Sichtenmodell (Clements et al.) und nenne die Ziel-Stakeholder.",
        "Da Logical, Process und Development schwer trennbar sind, fusioniert Clements sie zu:\n"
        "• <b>Component & Connector (C&C):</b> Zeigt dynamischen Daten- und Kontrollfluss. Komponenten (Objekte/Prozesse) und Konnektoren (Sockets, Shared Memory, JDBC).\n"
        "→ <i>Stakeholder:</i> Architekten, Entwickler, Kunden, Anwender.\n"
        "• <b>Module:</b> Statische Modularisierung (Strukturen, Klassen, Interfaces, Pakete).\n"
        "→ <i>Stakeholder:</i> Architekten, Entwickler.\n"
        "• <b>Allocation:</b> Zuweisung von SW auf HW (Deployment), SW auf Dateien (Implementierung) und SW auf Teams (Arbeitspakete).\n"
        "→ <i>Stakeholder:</i> Alle (Devs, SysAdmins, PMs).\n"
        "• <b>Use Cases (+1)</b> als roter Faden."
    ),

    # ===== 3. Wie beschreibt man Architektur (Komponenten & Konnektoren) =====
    (
        "Wann sind zwei Komponenten A und B abhängig? Was ist das Ziel bezüglich Abhängigkeiten und warum?",
        "<b>Abhängigkeit:</b>\n"
        "Zwei Komponenten A und B sind abhängig ($A \\rightarrow B$), wenn eine Änderung in B eine Änderung in A nach sich ziehen kann.\n\n"
        "<b>Ziel:</b>\n"
        "Minimierung der Abhängigkeiten (lose Kopplung, z. B. durch eine entkoppelnde Zwischenschicht).\n\n"
        "<b>Warum?</b>\n"
        "Viele Abhängigkeiten machen Änderungen, Testen, Bauen und die Abstimmung zwischen Teams extrem aufwendig und fehleranfällig."
    ),
    (
        "Wie definieren Szyperski und Meyer eine Komponente? Welche UML-Elemente beschreiben sie?",
        "• <b>Szyperski:</b> Ein kontextabhängiger Softwarebaustein mit definierten Schnittstellen.\n"
        "• <b>Meyer:</b> Modulare Einheit, die von Clients verwendbar ist, eine ausreichende Schnittstelle besitzt und nicht an einen vorgegebenen Client gebunden ist.\n\n"
        "<b>UML-Elemente:</b>\n"
        "• <b>Provided Interface (Lollipop &minus;&cir;):</b> Schnittstelle, die angeboten wird.\n"
        "• <b>Required Interface (Socket &minus;&sub;):</b> Schnittstelle, die benötigt wird.\n"
        "• <b>Port:</b> Dedizierter Interaktionspunkt mit der Umgebung (Menge von bereitgestellten/benötigten Interfaces)."
    ),
    (
        "Nenne die 4 Konnektor-Arten in UML und ihre Funktion.",
        "1. <b>Delegate-Connector:</b> Verbindet einen äußeren Port mit einem inneren Port (Weiterleitung in zusammengesetzten Komponenten).\n"
        "2. <b>Assembly-Connector:</b> Verbindet ein Required- mit einem Provided-Interface zweier Komponenten (&minus;&cir;&sub;&minus;).\n"
        "3. <b>Kompositionsstruktur:</b> Dedizierter Konnektor zwischen zwei Ports.\n"
        "4. <b>Assoziation (bzw. Assoziationsklasse):</b> Konnektor im Klassendiagramm (z. B. ein Puffer-Objekt)."
    ),
    (
        "Welche Regeln gelten für die Modellierung und Semantik von Konnektoren?",
        "• <b>Explizite Modellierung:</b> Konnektoren müssen explizit modelliert werden (keine impliziten verstecken). Sie brauchen Namen, Spezifikation (synchron/asynchron, FIFO/LIFO, Protokoll, Technologie) und NFR-Angaben.\n"
        "• <b>Goldene Regel:</b> Ein Konnektor verändert die Daten nicht (Semantik bleibt erhalten: $Semantik(A) \\Leftrightarrow Semantik(A')$).\n"
        "<i>Hinweis:</i> Kontextabhängig kann bei mobiler Übertragung eine Konvertierung/Adaption nötig sein."
    ),
    (
        "Was sind ADLs (Architecture Description Languages) und was bieten sie?",
        "<b>ADL (z. B. ACME):</b>\n"
        "Spezifische Modellierungssprachen zur Architekturbeschreibung.\n\n"
        "<b>Sie bieten:</b>\n"
        "• Ein einheitliches Austauschformat für interoperable Werkzeuge.\n"
        "• Ein formales Framework.\n"
        "• Einfache Sprachkonstrukte für Komponenten (Ports) und Konnektoren (Rollen, Attachments)."
    ),

    # ===== 4. Anforderungen & Qualitätsszenarien =====
    (
        "Was unterscheidet funktionale Anforderungen von nicht-funktionalen Anforderungen (NFR)?",
        "• <b>Funktionale Anforderungen:</b> Legen den Umfang der Lösung fest (<b>was</b> man bekommt). Sie sind <b>objektiv</b> (vorhanden oder nicht).\n"
        "• <b>Nicht-funktionale Anforderungen (NFR):</b> Definieren die Qualität der Lösung (<b>wie gut</b> es ist). Sie sind <b>subjektiv</b> (für jeden anders).\n\n"
        "<b>Umwandlung:</b> Über Metriken werden NFRs in überprüfbare, (pseudo-)funktionale Anforderungen verwandelt (z. B. \"nach 3 Fehlversuchen Account sperren\" statt \"System soll sicher sein\")."
    ),
    (
        "Nenne die Formeln für Verfügbarkeit und MTBF (Mean Time Between Failures).",
        "<b>Verfügbarkeit (Availability):</b>\n"
        "\\[ \\text{Verfügbarkeit} = \\frac{\\text{Gesamtzeit} - \\text{Downtime}}{\\text{Gesamtzeit}} \\]\n\n"
        "<b>MTBF (Zuverlässigkeit):</b>\n"
        "\\[ \\text{MTBF} = \\frac{\\text{Gesamtzeit}}{\\text{Anzahl Fehler}} \\]\n"
        "Beschreibt die mittlere Betriebsdauer zwischen Fehlern."
    ),
    (
        "Aus welchen 6 Elementen besteht ein Quality-Attribute-Szenario nach Clements/Bass/Kazman?",
        "1. <b>Quelle (Source of Stimulus):</b> Akteur/Komponente, die das Ereignis auslöst (z. B. Benutzer, fehlerhafter Filter).\n"
        "2. <b>Stimulus:</b> Das Ereignis, auf das reagiert werden muss (z. B. Systemausfall, Klick).\n"
        "3. <b>Artefakt:</b> Der betroffene Teil des Systems (z. B. Datenbank, Web-Server).\n"
        "4. <b>Environment:</b> Die Umgebung/der Lastzustand (z. B. Normalbetrieb, Überlast).\n"
        "5. <b>Antwort (Response):</b> Das Verhalten des Systems unter dem Stimulus.\n"
        "6. <b>Maße/Metriken (Response Measure):</b> Die Messgröße zur Bewertung (z. B. Antwortzeit < 1s, Recovery-Dauer)."
    ),
    (
        "Erkläre die Kern-Concerns und Maße von Verfügbarkeit und Modifizierbarkeit.",
        "• <b>Verfügbarkeit (Availability):</b>\n"
        "  - <i>Concerns:</i> Downtime (Wartung, Ausfälle), Time to Repair, Time to Recover.\n"
        "  - <i>Maße:</i> MTBF, Wiederherstellungszeit, Ausfallwahrscheinlichkeit.\n"
        "• <b>Modifizierbarkeit (Maintainability):</b>\n"
        "  - <i>Concerns:</i> Aufwand zur Anpassung (wer, was, wann, wo verursacht Änderung?).\n"
        "  - <i>Maße:</i> Kosten (€), Zeit (Manntage), Code-Umfang (LOC)."
    ),
    (
        "Was versteht man unter Skalierbarkeit und warum ist sie eine architektonische Herausforderung?",
        "<b>Skalierbarkeit:</b>\n"
        "Verhalten des Systems, wenn sich eine Dimension des Problemraums ausdehnt (z. B. Lastverhalten, simultane Verbindungen, Datengröße).\n\n"
        "<b>Herausforderung:</b>\n"
        "Sie ist anfangs oft keine \"offizielle\" Anforderung und Tests unter hoher Last sind sehr teuer. Wenn die Architektur nicht von Anfang an darauf ausgelegt ist, ist ein nachträglicher Einbau kaum möglich. Abhilfe: Auf bewährte Skalierungsmuster zurückgreifen."
    ),

    # ===== 5. Aufgaben des Architekten (Architektur-Zyklus, SAAM) =====
    (
        "Beschreibe die 4 Schritte des Architektur-Zyklus.",
        "1. <b>Requirements erfassen:</b> Aus funktionalen Anforderungen die NFRs extrahieren und in Kategorien einordnen (Performance, Security, Verfügbarkeit, Constraints).\n"
        "2. <b>Priorisieren:</b> Anforderungen in Stufen einteilen: <i>Hoch</i> (muss), <i>Mittel</i> (sollte), <i>Niedrig</i> (nice-to-have). Konflikte (z. B. Performance vs. Security) auflösen.\n"
        "3. <b>Entwurf:</b> Wahl der primären Architektur, Zerlegen in Komponenten, Festlegen der Konnektoren.\n"
        "4. <b>Validieren:</b> Szenarien durchspielen, Prototypen bauen."
    ),
    (
        "Was muss man beim Validieren einer Architektur beachten? (Klausur-Fokus)",
        "1. <b>Szenarien variieren:</b> Die Architektur beruht auf zentralen Szenarien. Zum Testen müssen diese variiert werden, da man sonst kaum Fehler findet.\n"
        "2. <b>Grenzen von Prototypen:</b> Prototypen sind für Skalierungs- und Performancefragen oft nur bedingt aussagekräftig. Im agilen Vorgehen kann am \"echten Produkt\" der ersten Sprints validiert werden."
    ),
    (
        "Nenne die 5 Phasen von SAAM (Software Architecture Analysis Method).",
        "SAAM dient der szenariobasierten Bewertung mehrerer Architekturkandidaten:\n"
        "1. <b>Szenarien erstellen</b>\n"
        "2. <b>Architekturkandidaten entwerfen</b>\n"
        "3. <b>Szenarien evaluieren</b>\n"
        "4. <b>Kandidaten bewerten & gewichten</b>\n"
        "5. <b>Architektur auswählen</b>"
    ),

    # ===== 6. Entwurfsprinzipien & SOLID & DDD =====
    (
        "Was ist ein Pattern (Muster)? Nenne Nutzen und Aufgaben von Architekturmustern.",
        "<b>Pattern:</b> Eine Relation zwischen Kontext (Situation), einem System widerstreitender Kräfte (Anforderungen) und einer bewährten Lösung für diesen Konflikt.\n\n"
        "<b>Nutzen:</b> Wiederverwendung bewährter Entwürfe, Dokumentation von Entscheidungen, Etablierung eines Standardvokabulars.\n\n"
        "<b>Aufgaben von Architekturmustern:</b> Vorlagen zur <i>Strukturierung</i> (C&C View: Zerlegung, Konnektoren) und zur <i>Implementierung</i> (Module View: Klassen/Interfaces, Kommunikation)."
    ),
    (
        "Erkläre die SOLID-Prinzipien (Robert C. Martin).",
        "• <b>S - Single Responsibility:</b> Jede Klasse/Komponente hat nur eine Verantwortlichkeit.\n"
        "• <b>O - Open/Closed:</b> Offen für Erweiterungen, geschlossen für Modifikationen (Vererbung/Polymorphie).\n"
        "• <b>L - Liskov Substitution:</b> Subklassen müssen Basisklassen problemlos ersetzen können.\n"
        "• <b>I - Interface Segregation:</b> Viele spezifische Schnittstellen statt weniger Riesen-Schnittstellen.\n"
        "• <b>D - Dependency Inversion:</b> Abhängigkeit von Abstraktionen (Interfaces), nicht von konkreten Implementierungen."
    ),
    (
        "Erkläre Domain-Driven Design (DDD) und vergleiche es kurz mit SOLID.",
        "<b>DDD (Eric Evans):</b>\n"
        "Die Fachdomäne ist Ausgangspunkt der Zerlegung. Erfordert enge Zusammenarbeit mit Fachbereich und eine gemeinsame Sprache (Ubiquitous Language).\n"
        "• <i>Isolating the Domain:</i> Fachliche Bereiche klar abgrenzen &rarr; hohe Kohäsion, lose Kopplung.\n"
        "• <i>Model in Software:</i> Fachkonzepte spiegeln sich in Packages/Klassen wider.\n"
        "• <i>Lifecycle:</i> Scopes festlegen (Application, Session, Request).\n\n"
        "<b>Vergleich:</b> SOLID ist technisch (Code-Struktur), DDD ist fachlich (Schnitt der Domäne)."
    ),

    # ===== 7. Layers, Onion & Hexagonal =====
    (
        "Was ist das Ziel der Schichtung (Layers)? Nenne die 3 klassischen Schichten und die Schichtungsregeln.",
        "<b>Ziel:</b> Trennung von Teilaufgaben unterschiedlicher Abstraktionsebenen.\n\n"
        "<b>Die 3 klassischen Schichten:</b>\n"
        "1. <b>Präsentationsschicht:</b> Zugriff von außen (UI, Web).\n"
        "2. <b>Logikschicht:</b> Eigentliche Geschäftslogik.\n"
        "3. <b>Persistenzschicht:</b> Anbindung an Drittsysteme (DB, Services).\n\n"
        "<b>Regeln:</b>\n"
        "• Höhere Schichten nutzen tiefere – niemals umgekehrt!\n"
        "• Partitionen auf gleicher Ebene sollten sich nicht gegenseitig nutzen.\n"
        "• Schnittstellen stabil halten. Blackbox-Zugriff (über Interface) ist Whitebox vorzuziehen."
    ),
    (
        "Wie funktioniert die Dynamik in Schichtenarchitekturen bei Informationsweitergabe nach oben?",
        "<b>Problem:</b>\n"
        "Eine tiefere Schicht N darf die höhere Schicht N+1 nicht kennen (Verbot von Zyklen).\n\n"
        "<b>Lösung:</b>\n"
        "• Delegierung erfolgt von oben nach unten (Methodenaufrufe).\n"
        "• Information nach oben erfolgt über <b>Callbacks</b> oder das <b>Observer-Muster</b>.\n"
        "• Die Schnittstellen für diese Rückmeldungen werden in der tieferen Schicht N definiert."
    ),
    (
        "Layers vs. Tiers: Was ist der Unterschied?",
        "• <b>Layers:</b> Logische Organisation des Codes (wie der Code strukturiert und getrennt ist).\n"
        "• <b>Tiers:</b> Physische Schichten (wo der Code ausgeführt wird, Deployment, Physical/Allocation View).\n\n"
        "<b>Beispiel:</b>\n"
        "Ein <i>Client Tier</i> (z. B. Browser) führt den Presentation Layer aus; ein <i>Business Tier</i> (App-Server) führt den Logic Layer aus; ein <i>EIS Tier</i> führt den Datenbank-Server aus."
    ),
    (
        "Vergleiche Application Server und Microservices in einer Tabelle.",
        "<table>"
        "<tr><th>Kriterium</th><th>Application Server</th><th>Microservices</th></tr>"
        "<tr><td><b>Grundidee</b></td><td>Klassische Schichten</td><td>Fachliche Trennung (DDD)</td></tr>"
        "<tr><td><b>Geschäftslogik</b></td><td>Zentraler Business Tier</td><td>Dezentrale Services</td></tr>"
        "<tr><td><b>Datenhaltung</b></td><td>Zentraler EIS Tier (eine DB)</td><td>Database per Service</td></tr>"
        "<tr><td><b>Kopplung</b></td><td>Eher stark</td><td>Gering</td></tr>"
        "<tr><td><b>Skalierung</b></td><td>Tier-weise (DB schwer)</td><td>Service-weise (komplexe Konsistenz)</td></tr>"
        "<tr><td><b>Deployment</b></td><td>Tier-weise</td><td>Service-spezifisch (CD)</td></tr>"
        "<tr><td><b>Kommunikation</b></td><td>Netzwerk + lokal im RAM</td><td>Ausschließlich Netzwerk</td></tr>"
        "</table>"
    ),
    (
        "Erkläre die Onion-Architektur (Palermo). In welche Richtung zeigen die Abhängigkeiten?",
        "<b>Onion-Struktur (von innen nach außen):</b>\n"
        "1. Domain Model (Kern)\n"
        "2. Process Layer (Use Cases)\n"
        "3. Application Services\n"
        "4. Peripherie (UI, DB, Tests, Frameworks)\n\n"
        "<b>Abhängigkeiten:</b>\n"
        "Zeigen immer <b>von außen nach innen</b>. Der Kern (die Domäne) ist absolut stabil und hängt von nichts ab. Peripherie hängt vom Kern ab. Kommunikation nach außen über Callbacks/Observer."
    ),
    (
        "Hexagonale Architektur (Ports & Adapter): Was unterscheidet \"Port als Vererbung\" von \"Port als Aggregation\"?",
        "• <b>Port als Vererbung:</b>\n"
        "Der Port ist das Interface (`PortA extends IAda, IBob`). Man legt sich früh auf eine Schnittstellenimplementierung fest.\n"
        "• <b>Port als Aggregation:</b>\n"
        "Man bezieht die Interfaces über den Port (`interface PortA { IAda iAda(); IBob iBob(); }`). Flexibler, da der Port als Fassade realisiert wird."
    ),

    # ===== 8. Blackboard-Architektur =====
    (
        "Was ist die Blackboard-Architektur? Wann setzt man sie ein?",
        "<b>schwarzes Brett (Blackboard):</b>\n"
        "Eigenständige Komponenten erarbeiten koordiniert eine Lösung auf einer geteilten Datenbasis. Keine direkte Kommunikation der Komponenten untereinander.\n\n"
        "<b>Einsatz:</b>\n"
        "Probleme ohne deterministische Lösung (KI, Expertensysteme, Spracherkennung). Bei unscharfem Wissen, wenn vollständiges Suchen zu teuer ist (exponentiell) und experimentell Zwischenergebnisse validiert werden müssen."
    ),
    (
        "Nenne die drei Kernkomponenten der Blackboard-Architektur und ihre Aufgaben.",
        "1. <b>Blackboard:</b> Zentrales Repository, verwaltet Hypothesen (Lösungsdaten) verschiedener Qualität und Kontrolldaten. Schnittstellen: `inspect()`, `update()`.\n"
        "2. <b>Knowledge Sources (Wissensquellen):</b> Unabhängige Spezialisten. Prüfen eigene Anwendbarkeit (`execCondition`) und berechnen/aktualisieren Hypothesen (`execAction` / `updateBlackboard`).\n"
        "3. <b>Control (Moderator):</b> Überwacht das Blackboard, wählt die nächste passende Wissensquelle aus und aktiviert sie (`loop`, `nextSource`)."
    ),
    (
        "Beschreibe den Control-Loop der Blackboard-Architektur und die Repository-Variante.",
        "<b>Control-Loop:</b>\n"
        "`nextSource` &rarr; `execCondition` (Quellen prüfen Blackboard auf Anwendbarkeit) &rarr; Auswahl &rarr; `execAction`/`updateBlackboard` &rarr; Wiederholung bis Problem gelöst.\n\n"
        "<b>Repository-Variante:</b>\n"
        "Blackboard ohne interne Kontrolle. Die Steuerung wird komplett von externen Anwendern/Anwendungen übernommen (z. B. IDE mit Plugins)."
    ),
    (
        "Nenne Vor- und Nachteile der Blackboard-Architektur.",
        "<b>Vorteile:</b>\n"
        "• Experimentierfreundlich.\n"
        "• Gute Änder- und Wartbarkeit (strikte Trennung von Daten, Kontrolle und Quellen).\n"
        "• Wiederverwendbarkeit der Wissensquellen.\n"
        "• Fehlertolerant.\n\n"
        "<b>Nachteile:</b>\n"
        "• Schwer zu testen (nicht-deterministisch, unscharfe Daten).\n"
        "• Keine Lösungsgarantie (oft wird nur ein Prozentsatz gelöst)."
    ),

    # ===== 9. Pipes & Filters & Mediator (EDA) =====
    (
        "Was ist das Pipes & Filters Muster? Nenne die übergeordneten Ziele.",
        "<b>Pipes & Filters:</b>\n"
        "Strukturiert Systeme, die Daten Schritt für Schritt verarbeiten. Jeder Verarbeitungsschritt ist ein eigenständiger Filter; die Konnektoren heißen Pipes.\n\n"
        "<b>Ziele:</b>\n"
        "• Kontinuierliche Workflow-Verarbeitung.\n"
        "• Einfache Erweiterung/Austausch von Filtern.\n"
        "• Filter arbeiten seiteneffektfrei (hängen nur von der Eingabe ab).\n"
        "• Parallele Verarbeitung (Filter arbeiten zeitgleich)."
    ),
    (
        "Pipes & Filters: Was unterscheidet aktive von passiven Filtern und Pull- von Push-Verarbeitung?",
        "• <b>Passiver Filter:</b> Wird von außen getriggert.\n"
        "  - <i>Pull:</i> Nachfolger entnimmt Daten (`read()`).\n"
        "  - <i>Push:</i> Vorgänger übergibt Daten (`send()`).\n"
        "• <b>Aktiver Filter:</b> Läuft als eigener Thread, holt sich selbst Daten aus der Pipe und schreibt sie weiter (Push-Pull-Kombination)."
    ),
    (
        "Pipes & Filters: Was sind implizite/explizite Pipes, Tee-and-Join und globale Daten?",
        "• <b>Implizite Pipes:</b> Direkter Methodenaufruf/RPC (kaum Parallelität, unflexibel).\n"
        "• <b>Explizite Pipes:</b> Eigene Kommunikationskanäle (Message Queues, Shared Memory); echte Parallelität, aber hoher Aufwand.\n"
        "• <b>Tee-and-Join:</b> Filter mit mehreren Ein- (Join) und Ausgängen (Tee) &rarr; gerichteter Graph.\n"
        "• <b>Globale Daten:</b> Zur Performance-Optimierung werden nur Referenzen auf Datencontainer statt der Daten selbst durchgereicht."
    ),
    (
        "Nenne Vor- und Nachteile von Pipes & Filters.",
        "<b>Vorteile:</b>\n"
        "• Keine Zwischendateien nötig.\n"
        "• Tees ermöglichen flexible Verzweigungen.\n"
        "• Filter sind leicht austauschbar und wiederverwendbar.\n"
        "• Effizienzgewinn durch Parallelität.\n\n"
        "<b>Nachteile:</b>\n"
        "• Gemeinsame Zustandsinfos sind teuer/komplex.\n"
        "• Fehlerbehandlung ist schwer (erfordert separaten Fehlerkanal).\n"
        "• Eventuell teure Datentransformationen zwischen Filtern."
    ),
    (
        "Wie funktioniert das Mediator-Muster in Event-Driven Architectures (EDA)? Nenne Vor- und Nachteile.",
        "Ein <i>Tee-and-Join-Pipeline-System</i> mit expliziten Pipes. Der **Mediator** teilt komplexe Events auf und verteilt sie asynchron über spezielle Pipes an **Event-Prozessoren** (kapseln Logik).\n\n"
        "<b>Vorteile:</b>\n"
        "• Sehr gute Performance & Skalierbarkeit.\n"
        "• Hohe Anpassungsfähigkeit, lose Kopplung.\n\n"
        "<b>Nachteile:</b>\n"
        "• Starke Abhängigkeit zwischen Mediator und Prozessoren.\n"
        "• Neue Prozessoren erfordern oft eine Anpassung des Mediators."
    ),

    # ===== 10. Middleware & MOM =====
    (
        "Was ist eine Middleware und welche Eigenschaften besitzt sie?",
        "<b>Middleware:</b>\n"
        "Die Abstraktionsebene zwischen Anwendung und realer Welt (OS, Cloud, HW).\n\n"
        "<b>Eigenschaften:</b>\n"
        "• Unabhängig von der Applikation.\n"
        "• Regelt die Topologie (1-1, 1-n, n-m).\n"
        "• Bietet einheitliche Kommunikation und Laufzeitumgebung.\n"
        "• Arbeitet transparent für den Anwender.\n"
        "• Reduziert <i>nicht</i> den Kommunikationsaufwand."
    ),
    (
        "Nenne die Middleware-Abstraktionsebenen gestern vs. heute.",
        "<table>"
        "<tr><th>Ebene</th><th>Gestern</th><th>Heute</th></tr>"
        "<tr><td><b>Kommunikation</b></td><td>OOM/RPC, CORBA, .NET Remote</td><td>HTTP, gRPC, Kafka, MOM</td></tr>"
        "<tr><td><b>Laufzeit/Services</b></td><td>Schwere App-Server (JEE)</td><td>Leichtgewichtig (Spring Boot, Node.js)</td></tr>"
        "<tr><td><b>Integration</b></td><td>Message Broker (ESB)</td><td>Docker-Container</td></tr>"
        "<tr><td><b>Orchestrierung</b></td><td>Business Process (BPEL)</td><td>Kubernetes</td></tr>"
        "</table>"
    ),
    (
        "Vergleiche objektorientierte (OOM) und ressourcenorientierte (REST/gRPC) Kommunikation.",
        "<table>"
        "<tr><th>Merkmal</th><th>Klassisch: OOM</th><th>Modern: REST/gRPC</th></tr>"
        "<tr><td><b>Ausführung</b></td><td>Zustandsbasiert</td><td>Zustandslos (funktional)</td></tr>"
        "<tr><td><b>Modell</b></td><td>Rein objektorientiert</td><td>Service-/ressourcenorientiert</td></tr>"
        "<tr><td><b>Aufrufe</b></td><td>I.d.R. synchron</td><td>Synchron oder asynchron</td></tr>"
        "<tr><td><b>Konfiguration</b></td><td>Hochkomplex (IDL, ORB)</td><td>Einfache Protokolle/JSON</td></tr>"
        "<tr><td><b>Transferkosten</b></td><td>Hoch</td><td>Geringer</td></tr>"
        "</table>"
    ),
    (
        "Was ist eine Message-Oriented Middleware (MOM) und welche Aufgaben hat der MOM-Server?",
        "<b>MOM:</b>\n"
        "Anwendungen sind autonom und kommunizieren asynchron über Nachrichten in Warteschlangen (Queues/Topics). Entkoppelt Sender und Empfänger vollständig.\n\n"
        "<b>Aufgaben MOM-Server:</b>\n"
        "• Nachrichten annehmen und in die korrekte Warteschlange sortieren.\n"
        "• Bestätigung (Ack) an den Sender schicken.\n"
        "• Nachrichten vorhalten, bis sie vom Empfänger abgeholt werden (oder ablaufen).\n"
        "• Auslieferung der Nachrichten."
    ),
    (
        "Nenne und erkläre die drei QoS-Stufen von MOM bezüglich Reliability vs. Performance.",
        "1. <b>Best Effort:</b>\n"
        "Nachrichten liegen nur im RAM. Sehr hohe Performance, aber Datenverlust bei Serverabsturz.\n"
        "2. <b>Persistent:</b>\n"
        "Nachrichten werden nicht-flüchtig auf Festplatte gesichert. Auslieferung auch nach Systemausfall garantiert. Geringere Performance.\n"
        "3. <b>Transactional:</b>\n"
        "Mehrere Nachrichten werden transaktionsgesichert (ACID) verarbeitet. Höchste Sicherheit, geringste Performance."
    ),
    (
        "Wie verhalten sich die Quality Attributes bei Verwendung einer MOM?",
        "• <b>Verfügbarkeit:</b> Hoch (Queues können physisch repliziert werden &rarr; Failover).\n"
        "• <b>Modifizierbarkeit:</b> Sehr hoch (Sender/Empfänger sind vollständig entkoppelt; nur das Nachrichtenschema koppelt).\n"
        "• <b>Performance:</b> Stark QoS-abhängig (RAM vs. Disk). Netzwerk-Engpässe beachten.\n"
        "• <b>Skalierbarkeit:</b> Sehr gut (Dedizierte MOM-Maschinen, Queues replizierbar)."
    ),
    (
        "Was besagt Fowlers \"First Law of Distributed Object Design\"? Was bedeutet das für Interfaces?",
        "<b>First Law:</b>\n"
        "<i>\"Don’t distribute your objects!\"</i> (Verteile deine Objekte nicht!). Jede Netzwerkgrenze erzeugt Latenz und Ausfallrisiko.\n\n"
        "<b>Konsequenz für Interfaces:</b>\n"
        "• <i>Lokales Interface:</i> Kann feingranular sein (viele Getter/Setter).\n"
        "• <i>Remote Interface:</i> Muss <b>grobgranular</b> sein. Wenige Aufrufe, die Daten komprimiert in DTOs (Data Transfer Objects) übertragen, um Roundtrips zu minimieren."
    ),

    # ===== 11. Patterns für den Business Layer =====
    (
        "GRASP (Larman): Was ist der Unterschied zwischen Doing und Knowing? Nenne die drei Zuweisungsregeln.",
        "<b>Verantwortlichkeiten bei GRASP:</b>\n"
        "• <b>Doing (Tun):</b> Etwas selbst tun, ein Objekt erzeugen oder Aktionen delegieren.\n"
        "• <b>Knowing (Wissen):</b> Eigene Daten/Zustand kennen, abgeleitete Daten berechnen oder assoziierte Objekte kennen.\n\n"
        "<b>Zuweisungsregeln:</b>\n"
        "1. <b>Experte:</b> Verantwortung der Klasse zuweisen, die die Informationen besitzt.\n"
        "2. <b>Lose Kopplung:</b> Abhängigkeiten minimieren.\n"
        "3. <b>Hohe Kohäsion:</b> Klassen fokussiert halten (nicht ein Gott-Objekt alles tun lassen)."
    ),
    (
        "Beschreibe Idee, Vor- und Nachteile von Transaction Script und Domain Model.",
        "• <b>Transaction Script:</b>\n"
        "  - <i>Idee:</i> Logik als Prozeduren/Methoden je Request/Use Case. Direkter DB-Zugriff.\n"
        "  - <i>Vorteil:</i> Geringer Grundaufwand, einfach zu verstehen.\n"
        "  - <i>Nachteil:</i> Code-Verdopplung, verliert bei Komplexität schnell die Übersicht.\n"
        "• <b>Domain Model:</b>\n"
        "  - <i>Idee:</i> Simulation des Problemraums (Daten + Verhalten in Objekten vereint).\n"
        "  - <i>Vorteil:</i> Kapselung, gut testbar, skaliert flach bei hoher Komplexität.\n"
        "  - <i>Nachteil:</i> Hoher initialer Grundaufwand (Mapping, Struktur)."
    ),
    (
        "Was ist das Table Module Pattern? Vergleiche es mit den anderen Business-Patterns.",
        "<b>Table Module:</b>\n"
        "Datenbankorientiert. Eine Klasse kapselt die gesamte Geschäftslogik für eine komplette Tabelle oder View (statt ein Objekt pro Zeile wie im Domain Model).\n\n"
        "<b>Vergleich (Fowlers Aufwandskurve):</b>\n"
        "• Bei <i>geringer Komplexität</i> ist <b>Transaction Script</b> am effizientesten, steigt aber exponentiell an.\n"
        "• Bei <i>hoher Komplexität</i> ist <b>Domain Model</b> am effizientesten (hoher Grundaufwand, flacher Anstieg).\n"
        "• <b>Table Module</b> liegt dazwischen und eignet sich gut für stark datenorientierte Systeme."
    ),

    # ===== 12. Zwischen Architektur und Design: Interfaces =====
    (
        "Was ist ein Proxy? Nenne 5 Proxy-Arten und ihr Zweck.",
        "<b>Proxy:</b>\n"
        "Ein Stellvertreter/Platzhalter für ein echtes Objekt (Subject) zur Zugriffskontrolle. Proxy und Subject implementieren dasselbe Interface.\n\n"
        "<b>Proxy-Arten:</b>\n"
        "1. <b>Virtueller Proxy:</b> Lazy Evaluation (echtes Objekt erst bei Zugriff erzeugen).\n"
        "2. <b>Schutz-Proxy:</b> Prüft Berechtigungen vor dem Zugriff.\n"
        "3. <b>Cache-Proxy:</b> Speichert Ergebnisse, um Berechnungen/Netzwerklast zu senken.\n"
        "4. <b>Synchronisations-Proxy:</b> Macht Aufrufe thread-safe.\n"
        "5. <b>Reverse Proxy:</b> Vertritt den Server vor den Clients (Caching, SSL, Load Balancing)."
    ),
    (
        "Was ist eine Bridge (Brücke)? Nenne ein konkretes Beispiel.",
        "<b>Bridge:</b>\n"
        "Trennt eine Abstraktion (Interface) von ihrer Implementierung, sodass beide unabhängig voneinander variieren können. Abstraktion delegiert an Implementor-Objekt.\n\n"
        "<b>Beispiel (Symboltabelle):</b>\n"
        "• Abstraktion: `Dictionary` (bietet `insert` / `lookup`).\n"
        "• Implementierung: `DictionaryImpl` (z. B. konkrete Implementierung über Hashtable `DictionaryHtab` oder Baum).\n"
        "Das Dictionary delegiert die Aufrufe an `DictionaryImpl`, welche im Hintergrund ausgetauscht werden kann."
    ),
    (
        "Was ist ein Adapter? Nenne zwei Varianten.",
        "<b>Adapter:</b>\n"
        "Passt eine inkompatible Schnittstelle an ein vom Client erwartetes Interface an, damit inkompatible Komponenten zusammenarbeiten können.\n\n"
        "<b>Varianten:</b>\n"
        "• <b>Zweiweg-Adapter:</b> Implementiert und bietet beide Schnittstellen an (Ziel und Adaptee).\n"
        "• <b>Pluggable-Adapter:</b> Schnittstelle wird dynamisch zur Laufzeit gebunden (z. B. über Reflection oder Funktionszeiger)."
    ),
    (
        "Vergleiche die Entwurfsmuster Proxy, Adapter und Bridge (Intuition).",
        "• <b>Proxy:</b> Behält das <b>gleiche Interface</b> bei. Er kontrolliert und steuert lediglich den Zugriff auf das reale Objekt.\n"
        "• <b>Adapter:</b> <b>Ändert das Interface</b>. Er konvertiert eine Schnittstelle in eine andere, um Inkompatibilitäten zu beheben.\n"
        "• <b>Bridge:</b> Ist von Anfang an so entworfen, das <b>Interface und die Implementierung getrennt</b> sind, damit beide unabhängig wachsen können."
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = 'SWA_Klausurwissen.apkg'
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' mit {len(flashcards)} Karten.")