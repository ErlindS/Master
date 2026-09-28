import genanki

MODEL_ID = 1829384780
DECK_ID = 2938475640

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
    'ITES Clean Model 030',
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
    'IT-Entrepreneurship :: 030 Geschäftsidee & Geschäftsmodell'
)

flashcards = [
    (
        "Welche 4 Phasen durchläuft ein Gründer auf dem Weg von der Idee zur Führung eines Unternehmens?",
        "<b>Der Entwicklungsweg:</b>\n"
        "1. <b>Geschäftsidee:</b> Der erste Einfall / das Konzept.\n"
        "2. <b>Geschäftsmodell:</b> Die Ausgestaltung des Werts und Ertrags.\n"
        "3. <b>Unternehmensgründung:</b> Der formelle Gründungsakt.\n"
        "4. <b>Unternehmensführung:</b> Der laufende Betrieb."
    ),
    (
        "Aus welchen 3 Hauptkomponenten besteht ein Geschäftsmodell nach Patrick Stähler?",
        "1. <b>Architektur der Wertschöpfung:</b> Die logische Funktionsweise und operativen Prozesse des Unternehmens.\n"
        "2. <b>Nutzenversprechen (Value Proposition):</b> Wie genau ein Mehrwert oder Nutzen für den Kunden erzeugt wird.\n"
        "3. <b>Ertragsmodell:</b> Wie Gewinne erzielt werden (besteht aus dem Erlösmodell und dem Kostenmodell)."
    ),
    (
        "Welche zwei zentralen Methoden werden zur Beschreibung und Entwicklung von Geschäftsmodellen genutzt?",
        "1. <b>Businessplan</b>\n"
        "2. <b>Business Model Canvas (BMC)</b>"
    ),
    (
        "Nenne die typischen Eigenschaften eines Businessplans (Umfang, Detailtiefe, Dynamik, Aufwand).",
        "• <b>Umfang:</b> Ausführliche schriftliche Beschreibung des Unternehmens (ca. 20–50 Seiten).\n"
        "• <b>Detailtiefe:</b> Sehr hoch bezüglich prognostizierter Finanz- und Absatzzahlen.\n"
        "• <b>Dynamik:</b> Relativ starres Instrument.\n"
        "• <b>Aufwand:</b> Zeit- und kostenintensiv (mehrere Wochen Arbeit; erfordert oft kostenpflichtige Prüfung/Unterschrift durch Steuerberater)."
    ),
    (
        "Wofür wird ein formeller Businessplan zwingend benötigt (3 Beispiele)?",
        "1. <b>Fördermittel-Anträge</b> (z. B. Gründungszuschuss der Arbeitsagentur für ALG I-Bezieher).\n"
        "2. <b>Verhandlung von Bankkrediten</b>.\n"
        "3. <b>Ansprache traditioneller Investoren</b>."
    ),
    (
        "Welche 11 klassischen Gliederungspunkte umfasst ein Businessplan nach BMWi-Standard?",
        "1. Zusammenfassung (Executive Summary)\n"
        "2. Gründerperson(en)\n"
        "3. Geschäftsidee (Produkt/Dienstleistung)\n"
        "4. Entwicklungsintensive Vorhaben (Entwicklungsmilestones, Patente)\n"
        "5. Markt und Wettbewerb (Kunden, Konkurrenz, Standort)\n"
        "6. Marketing (Preismodelle, Vertrieb)\n"
        "7. Organisation und Mitarbeiter\n"
        "8. Rechtsform\n"
        "9. Risiken und Chancen (z. B. SWOT-Analyse)\n"
        "10. Finanzplan (Liquiditätsplan, Kapitalbedarf)\n"
        "11. Anlagen (Lebensläufe, Gesellschaftsvertrag)"
    ),
    (
        "Wofür steht die Abkürzung SWOT im Rahmen der Risikoanalyse eines Businessplans?",
        "Die SWOT-Analyse bewertet interne und externe Faktoren:\n"
        "• <b>S</b>trengths (Stärken) – intern\n"
        "• <b>W</b>eaknesses (Schwächen) – intern\n"
        "• <b>O</b>pportunities (Chancen) – extern\n"
        "• <b>T</b>hreats (Risiken) – extern"
    ),
    (
        "Was ist das Business Model Canvas (BMC) und welches sind seine Haupteigenschaften?",
        "• <b>Definition:</b> Ein visuelles Instrument zur schnellen Entwicklung und Überarbeitung innovativer und komplexer Geschäftsmodelle.\n"
        "• <b>Ansatz:</b> Brainstorming-orientiert (Arbeit mit Haftnotizen auf einem großen Plakat).\n"
        "• <b>Einsatzgebiete:</b> Lean Startups, Bootcamps (zur schnellen Überprüfung von Geschäftsideen), Investoren-Präsentationen, Vorstufe zum Businessplan."
    ),
    (
        "In welche 4 übergeordneten Bereiche lassen sich die 9 Faktoren des Business Model Canvas einteilen?",
        "1. <b>Kunden</b> (Wer?)\n"
        "2. <b>Nutzenversprechen</b> (Was?)\n"
        "3. <b>Aktivitäten & Partner</b> (Wie?)\n"
        "4. <b>Finanzielle Realisierbarkeit</b> (Wie viel?)"
    ),
    (
        "Nenne die 9 Schlüsselfaktoren (Elemente) des Business Model Canvas.",
        "1. Schlüssel-Partner (Key Partners)\n"
        "2. Schlüssel-Aktivitäten (Key Activities)\n"
        "3. Nutzenversprechen (Value Propositions)\n"
        "4. Kundenbeziehungen (Customer Relationships)\n"
        "5. Kundensegmente / Kunden-Arten (Customer Segments)\n"
        "6. Schlüssel-Ressourcen (Key Resources)\n"
        "7. Kanäle / Vertrieb & Kommunikation (Channels)\n"
        "8. Kostenstruktur (Cost Structure)\n"
        "9. Einnahmequellen (Revenue Streams)"
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Schlüssel-Partner' im BMC?",
        "<b>Wer kommt als Partner oder Lieferant in Frage?</b>\n"
        "Es geht darum, welche strategischen Allianzen, Partnerschaften mit Konkurrenten oder Lieferantenbeziehungen nötig sind, um Risiken zu minimieren oder Ressourcen zu teilen."
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Schlüssel-Aktivitäten' im BMC?",
        "<b>Welches sind die wichtigsten Tätigkeiten, um dieses Geschäftsmodell in die Tat umzusetzen?</b>\n"
        "Dazu gehören alle operativen und strategischen Prozesse, die notwendig sind, um das Nutzenversprechen zu erstellen und aufrechtzuerhalten."
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Nutzenversprechen' (Value Propositions) im BMC und was bedeutet USP?",
        "• <b>Leitfrage:</b> Welchen Nutzen haben die Kunden, wenn sie das Produkt oder die Dienstleistung kaufen? Welches Problem lösen wir für sie?\n"
        "• <b>USP (Unique Selling Proposition):</b> Das Alleinstellungsmerkmal, das das Angebot einzigartig macht und von Mitbewerbern abhebt."
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Kundenbeziehungen' im BMC?",
        "<b>Wie können die in Frage kommenden Kunden gewonnen und gebunden werden?</b>\n"
        "Wie gestalten wir die Interaktion mit dem Kunden (z. B. persönliche Betreuung, automatisierte Self-Services, Co-Creation, Communities)?"
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Kundensegmente' (Kunden-Arten) im BMC?",
        "<b>Wer ist die Zielgruppe des Geschäftsmodells?</b>\n"
        "Welche Segmente wollen wir bedienen (z. B. Massenmarkt, Nischenmarkt, B2B, B2C, multi-sided markets)?"
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Schlüssel-Ressourcen' im BMC?",
        "<b>Welche Ressourcen sind unverzichtbar, um das Geschäftsmodell umzusetzen?</b>\n"
        "Dazu gehören physische Mittel (Betriebsstätte, Maschinen), intellektuelle Werte (Marken, Patente, Lizenzen), menschliche Ressourcen (Fachpersonal) oder finanzielle Mittel (Startkapital)."
    ),
    (
        "Welche Leitfragen verbergen sich hinter dem Element 'Kanäle' (Channels) im BMC?",
        "<b>Wie erfahren Kunden von dem Angebot? Wie muss der Vertrieb aussehen?</b>\n"
        "Es beschreibt die Kommunikations-, Vertriebs- und Verkaufswege, über die ein Unternehmen mit seinen Kunden in Kontakt tritt."
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Kostenstruktur' im BMC?",
        "<b>Welches sind die wichtigsten Ausgaben, ohne die das Geschäftsmodell nicht funktionieren würde?</b>\n"
        "Hier werden die primären Kostentreiber identifiziert, die durch Aktivitäten, Ressourcen und Partner entstehen (z. B. fix vs. variabel, wertorientiert vs. kostenorientiert)."
    ),
    (
        "Welche Leitfrage verbirgt sich hinter dem Element 'Einnahmequellen' (Revenue Streams) im BMC?",
        "<b>Woher kommt bei diesem Geschäftsmodell das Geld?</b>\n"
        "Welchen Wert sind Kunden bereit zu zahlen und über welche Mechanismen (z. B. Einmalzahlungen, Abonnements, Nutzungsgebühren, Lizenzierung)?"
    ),
    (
        "Nenne die Stärken und Schwächen des Business Model Canvas (BMC).",
        "• <b>Stärken:</b> Sehr gutes Instrument zum Brainstorming, extrem schnell (erste Version in 20–30 Min.), hohe Flexibilität (Post-its lassen sich leicht verschieben/ersetzen) und gute Visualisierung.\n"
        "• <b>Schwächen:</b> Markt- und Wettbewerbssituation werden nur am Rande abgebildet, und es fehlt an quantitativer Detailtiefe (insbesondere im Vergleich zur Finanzplanung im Businessplan)."
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = '030_Geschaeftsidee_Geschaeftsmodell.apkg'
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' mit {len(flashcards)} Karten.")
