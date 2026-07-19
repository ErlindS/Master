import genanki

MODEL_ID = 1829384770
DECK_ID = 2938475630

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
    'ITES Clean Model 020',
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
    'IT-Entrepreneurship :: 020 Angestellter & Nebenerwerbsgründung'
)

flashcards = [
    (
        "Was sind die 3 rechtlichen Voraussetzungen für das Vorliegen einer Nebenerwerbsselbständigkeit?",
        "1. Eine <b>zweite, selbständige Erwerbstätigkeit</b> wird zusätzlich zur ersten Erwerbstätigkeit (abhängige Beschäftigung, Beamtentum etc.) in Teilzeit ausgeführt.\n"
        "2. Die Einkünfte aus der Nebentätigkeit tragen <b>weniger als die Hälfte</b> zum Gesamteinkommen bei (Richtwert: 2/3 Hauptberuf zu 1/3 Nebenberuf).\n"
        "3. Es besteht eine ernsthafte <b>Gewinnerzielungsabsicht</b> (keine steuerliche Liebhaberei)."
    ),
    (
        "Was sollte man als Angestellter bezüglich einer selbständigen Nebentätigkeit bei Arbeitsvertragsverhandlungen beachten?",
        "• <b>Offenes Ansprechen:</b> Plausible Argumente vorbereiten, warum die Haupttätigkeit nicht leidet oder der Nebenjob ihr sogar nutzt (z. B. selbstständiger \"Whitehat Hacker\" für Hauptjob in der IT-Security).\n"
        "• <b>Öffentlicher Dienst:</b> Die Nebentätigkeit muss angezeigt bzw. genehmigt werden.\n"
        "• <b>Freie Wirtschaft:</b> Es empfiehlt sich, die Genehmigung schriftlich im Arbeitsvertrag festzuhalten (z. B. \"Nebentätigkeiten im bisherigen Umfang werden gestattet\") oder dies zumindest per E-Mail bestätigen zu lassen."
    ),
    (
        "Unter welchen 3 Bedingungen ist eine selbständige Nebentätigkeit sozialversicherungsfrei (Stand 2021)?",
        "1. Es liegt eine <b>geringfügige Nebentätigkeit</b> vor (maximal 18 Stunden pro Woche).\n"
        "2. Der <b>Gewinn</b> (nicht Umsatz!) beträgt maximal 1.800 € p.a. (Für Studierende in der Familienversicherung gilt: mtl. Einkommensgrenze 505 € + 83,33 € Werbungskostenpauschale = max. 588,33 € mtl.).\n"
        "3. Es werden <b>keine nicht-geringfügigen Angestellten</b> beschäftigt (Minijobber mit < 538 € bzw. < 556 € ab 2025 sind zulässig)."
    ),
    (
        "Was bedeutet 'Liebhaberei' im Steuerrecht bei einer selbständigen Nebentätigkeit und welche Folge hat sie?",
        "• <b>Bedeutung:</b> Wenn ein Gewerbe oder eine freiberufliche Tätigkeit über mehrere Jahre hinweg nur Verluste abwirft und keine echte Gewinnerzielungsabsicht erkennbar ist (z. B. Hobby-Projekte).\n"
        "• <b>Folge:</b> Das Finanzamt stuft die Tätigkeit als Liebhaberei ein. Die steuerliche Absetzbarkeit von Verlusten wird rückwirkend aberkannt, und Steuervorteile müssen zurückgezahlt werden."
    ),
    (
        "Darf der Arbeitgeber eine Nebentätigkeit grundsätzlich verbieten? Wie ist die Rechtslage?",
        "<b>Nein, grundsätzlich nicht.</b> Nach dem Grundgesetz gilt das Recht auf freie Berufswahl. Eine Nebentätigkeit ist somit prinzipiell nicht genehmigungspflichtig durch den Arbeitgeber.\n"
        "Pauschale Verbotsklauseln in Arbeitsverträgen sind rechtlich <b>unzulässig</b>. Der Arbeitgeber kann eine Nebentätigkeit jedoch untersagen, wenn berechtigte betriebliche Interessen verletzt werden (z. B. Konkurrenzverbot, Arbeitszeitüberschreitungen)."
    ),
    (
        "Welche 7 Kriterien/Regeln muss ein Arbeitnehmer einhalten, damit der Arbeitgeber die Nebentätigkeit nicht verbieten kann?",
        "1. Die Nebentätigkeit beeinträchtigt die Leistung im Hauptjob nicht negativ.\n"
        "2. Dem Arbeitgeber entsteht <b>keine Konkurrenz</b> (Wettbewerbsverbot).\n"
        "3. Dem Ansehen des Arbeitgebers wird nicht geschadet.\n"
        "4. Die Nebentätigkeit überschreitet die wöchentliche Normalarbeitszeit um maximal 20 % (ca. 1 Tag/Woche).\n"
        "5. Krankheit oder Urlaub dürfen nicht für die Nebentätigkeit genutzt werden.\n"
        "6. Haupt- und Nebenjob müssen räumlich/organisatorisch klar getrennt sein.\n"
        "7. Die <b>gesetzliche Höchsarbeitszeit</b> darf nicht überschritten werden."
    ),
    (
        "Welche gesetzlichen Höchstarbeitszeiten gelten nach § 3 ArbZG und was bedeutet das für Vollzeitangestellte im Nebenerwerb?",
        "• <b>Gesetzliche Regelung:</b> Die werktägliche Arbeitszeit darf 8 Stunden nicht überschreiten (Samstag gilt als Werktag). In Ausnahmefällen sind bis zu 10 Stunden zulässig.\n"
        "• <b>Konsequenz bei Vollzeit (40 Std.):</b> Rechnerisch bleiben maximal <b>8 Stunden pro Woche</b> für die Nebentätigkeit übrig. Mehr Stunden sind rechtlich nur bei Teilzeitbeschäftigten in der Haupttätigkeit zulässig."
    ),
    (
        "Ab welchem Gewinn aus einer selbständigen Nebentätigkeit muss Einkommensteuer gezahlt werden (Stand 2024)?",
        "Ab einem Gewinn von <b>410 € p.a.</b> (Freigrenze, kein Freibetrag – bei Überschreitung muss der gesamte Gewinn versteuert werden).\n"
        "Der Gewinn wird über eine Einnahmenüberschussrechnung (EÜR) ermittelt und in der Anlage S (Freiberufler) bzw. Anlage G (Gewerbe) der Steuererklärung angegeben."
    ),
    (
        "Was ist der Übungsleiterfreibetrag und wie hoch ist er steuerfrei (Stand 2024)?",
        "Einnahmen aus nebenberuflichen Tätigkeiten als Ausbilder, Übungsleiter, Erzieher oder Dozent (z. B. Lehrauftrag an einer Hochschule) sind bis zu einem Gewinn von <b>3.000 € pro Kalenderjahr</b> komplett steuerfrei."
    ),
    (
        "Unter welchen Bedingungen kann man bei einer Nebentätigkeit in der gesetzlichen Krankenversicherung des Hauptjobs verbleiben?",
        "Man muss keine zusätzliche, eigene Krankenversicherung abschließen, sofern:\n"
        "1. die Nebentätigkeit die Haupttätigkeit <b>wirtschaftlich nicht überwiegt</b> (Gewinn der Nebentätigkeit vs. Nettoeinkommen des Hauptjobs: 2/3 zu 1/3 Richtwert).\n"
        "2. die Nebentätigkeit zeitlich maximal <b>8–10 Stunden pro Woche</b> ausgeübt wird.\n\n"
        "Andernfalls wird eine freiwillige gesetzliche oder private Krankenversicherung erforderlich. Gleiches gilt bei studentischer Familienversicherung (< 25 Jahre)."
    ),
    (
        "Gibt es für selbständige Softwareentwickler eine gesetzliche Rentenversicherungspflicht?",
        "<b>Nein.</b> Softwareentwickler sind im Gegensatz zu bestimmten anderen Berufsgruppen (wie Lehrern, Hebammen, Erziehern oder Handwerkern) grundsätzlich <b>nicht</b> rentenversicherungspflichtig."
    ),
    (
        "Welche wöchentlichen Arbeitszeitgrenzen gelten für die Nebentätigkeit je nach Institution?",
        "• <b>Gewerbe- oder Finanzamt:</b> Keine Grenze.\n"
        "• <b>Krankenversicherung:</b> Maximal 18 Stunden pro Woche (darüber gilt es als Hauptberuf).\n"
        "• <b>Arbeitgeber (bei Vollzeit):</b> Maximal 8 Stunden wöchentlich (gemäß Arbeitszeitgesetz)."
    ),
    (
        "Welche Hinzuverdienstgrenzen gelten für nebenberuflich selbstständige Arbeitnehmer?",
        "• <b>Finanzamt:</b> Keine Grenze. Es gibt keine Zuverdienstgrenze, jedoch muss der gesamte Gewinn versteuert werden.\n"
        "• <b>Krankenkasse/Arbeitgeber:</b> Die Nebentätigkeit darf die Haupttätigkeit wirtschaftlich nicht überwiegen (2/3 zu 1/3 Regel)."
    ),
    (
        "Nenne mindestens 4 Kriterien, bei deren Erfüllung man aus rechtlicher Sicht als scheinselbstständig eingestuft wird.",
        "Man gilt als scheinselbstständig, wenn 3 oder mehr der folgenden Punkte zutreffen:\n"
        "1. Keine sozialversicherungspflichtigen Angestellten (> 538 € bzw. 556 € ab 2025).\n"
        "2. Man ist überwiegend nur für <b>einen Kunden</b> tätig (Richtwert: > 84 % des Umsatzes).\n"
        "3. Keine freie Entscheidung über die Arbeitszeit.\n"
        "4. Keine freie Wahl des Arbeitsorts / fester Arbeitsplatz beim Kunden eingerichtet.\n"
        "5. Keine aktive Werbung um neue Kunden (fehlender Außenauftritt).\n"
        "6. Keine freie Festlegung von Preisen und Stundensätzen.\n"
        "7. Man verrichtet beim Kunden dieselbe Tätigkeit, die man dort zuvor als Angestellter getan hat."
    ),
    (
        "Welche Konsequenzen hat die Einstufung als Scheinselbstständiger für Auftraggeber und Auftragnehmer?",
        "• <b>Rückwirkende Behandlung:</b> Beide Parteien werden rückwirkend wie Arbeitnehmer/Arbeitgeber behandelt.\n"
        "• <b>Nachzahlungspflicht:</b> Sozialversicherungsbeiträge müssen für die letzten 6 Jahre (bei Vorsatz bis zu 30 Jahre) nachgezahlt werden. Der Auftraggeber haftet meist für die Beiträge des Scheinselbstständigen.\n"
        "• <b>Steuern:</b> Verlust des Betriebsausgabenabzugs über EÜR; Besteuerung als regulärer Arbeitnehmer."
    ),
    (
        "Warum ist Nils als Angestellter/Student an der Hochschule nicht scheinselbstständig, aber nach seinem Ausscheiden gefährdet?",
        "• <b>Während der Anstellung:</b> Der Status als Angestellter/Student sichert seine Sozialversicherung über die Haupttätigkeit ab, weshalb er nicht als scheinselbstständig eingestuft wird.\n"
        "• <b>Nach dem Ausscheiden:</b> Er arbeitet ausschließlich für einen Kunden (Soft AG), hat keine Angestellten und wirbt nicht aktiv um Kunden. Er erfüllt damit 3 Kriterien der Scheinselbstständigkeit und hat dringenden Handlungsbedarf."
    ),
    (
        "Welche 3 Maßnahmen kann ein Freiberufler ergreifen, um Scheinselbstständigkeit wirksam vorzubeugen?",
        "1. <b>Mitarbeiter einstellen:</b> Einen sozialversicherungspflichtigen Mitarbeiter beschäftigen (z. B. mit > 556 € mtl. für Softwaretests).\n"
        "2. <b>Kundenstamm erweitern:</b> Für mehrere unterschiedliche Unternehmen tätig sein (sodass kein Kunde > 84 % des Umsatzes ausmacht).\n"
        "3. <b>Außenauftritt pflegen:</b> Werbung schalten, Website betreiben, Angebote abgeben, um als eigenständiger Marktteilnehmer aufzutreten."
    ),
    (
        "Wie kann die Gründung einer Kapitalgesellschaft helfen, eine Scheinselbstständigkeit zu umgehen?",
        "Der Gründer tritt nicht mehr direkt als Person auf, sondern gründet eine Kapitalgesellschaft (UG oder GmbH) und wird dort als <b>geschäftsführender Gesellschafter</b> angestellt.\n"
        "Das Vertragsverhältnis mit Kunden verschiebt sich dadurch auf die reine B2B-Ebene (Unternehmen zu Unternehmen)."
    ),
    (
        "Was ist der Gründungszuschuss (Dauer, Höhe, Zuverdienst, Stand 2024)?",
        "Eine Förderung der Bundesagentur für Arbeit bei der Existenzgründung aus der Arbeitslosigkeit (ALG I):\n"
        "• <b>Dauer:</b> 6 Monate ab Gründung (Verlängerung um 9 Monate für Sozialabsicherung möglich).\n"
        "• <b>Höhe:</b> Entspricht dem bisherigen monatlichen ALG I-Bezug + 300 € Pauschale zur sozialen Absicherung.\n"
        "• <b>Zuverdienst:</b> Ist in unbegrenzter Höhe möglich, ohne dass der Gründungszuschuss gekürzt wird."
    ),
    (
        "Was ist das Statusfeststellungsverfahren, wo wird es beantragt und was wird empfohlen?",
        "• <b>Zweck:</b> Ein rechtsverbindliches Verfahren zur Feststellung, ob ein Auftragsverhältnis als abhängige Beschäftigung oder als selbstständige Tätigkeit einzustufen ist.\n"
        "• <b>Beantragung:</b> Bei der <b>Clearingstelle der Deutschen Rentenversicherung Bund (DRV)</b> innerhalb von 4 Wochen nach Aufnahme der Tätigkeit.\n"
        "• <b>Empfehlung:</b> Keine Einzelvertragsprüfung beantragen, sondern die Gesamtsituation (am besten mit mehreren Verträgen/Angeboten) einreichen."
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = '020_Angestellter_Nebenerwerbsgruendung.apkg'
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' mit {len(flashcards)} Karten.")
