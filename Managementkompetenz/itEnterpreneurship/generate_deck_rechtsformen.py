import genanki

MODEL_ID = 1829384760
DECK_ID = 2938475620

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
    'ITES Clean Model',
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
    'IT-Entrepreneurship :: 060 Gründung & Rechtsformen'
)

flashcards = [
    (
        "In welche zwei Hauptgruppen lassen sich die Rechtsformen (Unternehmensformen) unterteilen?",
        "<b>Die zwei Hauptgruppen:</b>\n"
        "1. <b>Personengesellschaften</b> (z. B. Einzelunternehmen, Kaufmann, Freiberufler, GbR, PartG, OHG, KG)\n"
        "2. <b>Kapitalgesellschaften</b> (z. B. UG, GmbH, AG, Ltd.)"
    ),
    (
        "Was sind die Merkmale eines Einzelunternehmens bezüglich Haftung, Buchführung, Namensführung und Handelsregister?",
        "• <b>Haftung:</b> Unbeschränkt persönlich mit dem kompletten Privatvermögen.\n"
        "• <b>Buchführung:</b> In der Regel Einnahmenüberschussrechnung (EÜR). Doppelte Buchführungspflicht erst ab 60 T€ Gewinn oder 600 T€ Umsatz (§ 141 AO).\n"
        "• <b>Namensführung:</b> Der eigene Vor- und Nachname muss im Geschäftsverkehr angegeben werden (reiner Phantasiename reicht nicht).\n"
        "• <b>Handelsregister:</b> Nein, kein Eintrag."
    ),
    (
        "Wann gilt man als eingetragener Kaufmann (e.K.) und welche Regelungen gelten bei Haftung, Buchführung und Handelsregister?",
        "• <b>Voraussetzung:</b> Wenn laut Gesetz \"nach Art oder Umfang ein in kaufmännischer Weise eingerichteter Geschäftsbetrieb\" benötigt wird (z. B. Ladengeschäft, Restaurant).\n"
        "• <b>Haftung:</b> Unbeschränkt persönlich mit dem kompletten Privatvermögen.\n"
        "• <b>Buchführung:</b> Doppelte Buchführung (nach HGB).\n"
        "• <b>Namensführung:</b> Eigener Firmenname (auch Phantasiename) mit dem Zusatz \"e.K.\" erlaubt.\n"
        "• <b>Handelsregister:</b> Ja, Eintragung zwingend."
    ),
    (
        "Können selbständige Freiberufler bzw. Freiberufler-Teams als Kaufleute agieren?",
        "<b>Nein.</b> Selbständige Freiberufler sowie Freiberufler-Teams in der Rechtsform einer Partnerschaftsgesellschaft (PartG) oder Gesellschaft bürgerlichen Rechts (GbR) gehören <b>nicht</b> zu den Kaufleuten.\n"
        "Sie werden nicht im Handelsregister eingetragen. Rechtliche Grundlage sind das BGB sowie berufsrechtliche Bestimmungen."
    ),
    (
        "Was zeichnet einen Freiberufler bezüglich Haftung, Buchführung und Handelsregister aus?",
        "• <b>Haftung:</b> Unbeschränkt persönlich mit dem kompletten Privatvermögen.\n"
        "• <b>Buchführung:</b> Einnahmenüberschussrechnung (EÜR) ohne Umsatz- oder Gewinngrenzen (freiwillige doppelte Buchführung möglich).\n"
        "• <b>Handelsregister:</b> Nein, kein Eintrag."
    ),
    (
        "Was zeichnet eine Gesellschaft bürgerlichen Rechts (GbR) aus? (Haftung, Buchführung, Besonderheit bei Ausscheiden)",
        "• <b>Eignung:</b> Für selbständige Gewerbetreibende oder Freiberufler, wenn sie mit mindestens einem Partner gründen.\n"
        "• <b>Haftung:</b> Persönlich mit dem eigenen Privatvermögen aller Gesellschafter, und zwar <b>gesamtschuldnerisch</b> (auch für die Fehler der anderen Partner!).\n"
        "• <b>Buchführung:</b> Einnahmenüberschussrechnung (EÜR).\n"
        "• <b>Handelsregister:</b> Nein.\n"
        "• <b>Besonderheit:</b> Beim Ausscheiden eines Gesellschafters erlischt die GbR (es wird dringend empfohlen, das Ausscheiden im Gesellschaftervertrag zu regeln)."
    ),
    (
        "Was ist eine Partnerschaftsgesellschaft (PartG) und welche Haftungsbesonderheit gilt hier für Freiberufler?",
        "• <b>Zweck:</b> Zusammenschluss mehrerer Freiberufler (der Freiberufler-Status bleibt für alle erhalten).\n"
        "• <b>Haftung:</b> Partner haften grundsätzlich mit ihrem Privatvermögen, aber <b>nur für die eigenen Fehler</b> (nicht für die Fehler der anderen Partner). Daher beliebt in Gemeinschaftspraxen, MVZs, etc.\n"
        "• <b>Buchführung:</b> Einnahmenüberschussrechnung (EÜR).\n"
        "• <b>Register:</b> Eintragung im Partnerschaftsregister."
    ),
    (
        "Was ist eine Offene Handelsgesellschaft (OHG) und welche Besonderheiten gelten bei Haftung und Auflösung?",
        "• <b>Definition:</b> Im Grunde die GbR für Kaufleute (Zusammenschluss mehrerer Kaufleute).\n"
        "• <b>Haftung:</b> Alle Gesellschafter haften persönlich mit ihrem Privatvermögen (auch für Fehler der anderen!).\n"
        "• <b>Buchführung:</b> Doppelte Buchführung.\n"
        "• <b>Handelsregister:</b> Ja.\n"
        "• <b>Besonderheit bei Ausscheiden:</b> Scheidet ein Partner bei zwei verbliebenen Gesellschaftern aus, erlischt die OHG und der letzte Gesellschafter kann als e.K. weitermachen."
    ),
    (
        "Was zeichnet eine Unternehmergesellschaft (UG haftungsbeschränkt) aus? (Kapital, Name, Haftung, Buchführung)",
        "• <b>Spitzname:</b> \"Mini-GmbH\" oder \"1-Euro-GmbH\" (deutsche Alternative zur britischen Ltd.).\n"
        "• <b>Stammkapital:</b> Ab 1 € (keine Sacheinlagen möglich). Ab 12.500 € Stammkapital kann sie in eine GmbH umgewandelt werden.\n"
        "• <b>Zusatz im Namen:</b> Zwingend \"UG (haftungsbeschränkt)\" (kann auf manche Geschäftspartner/Banken abschreckend wirken).\n"
        "• <b>Haftung:</b> Beschränkt auf das Gesellschaftsvermögen.\n"
        "• <b>Buchführung:</b> Doppelte Buchführung.\n"
        "• <b>Handelsregister:</b> Ja."
    ),
    (
        "Was sind die Merkmale einer GmbH? (Stammkapital, Sacheinlagen, Haftung, Buchführung)",
        "• <b>Gründer:</b> Sowohl gewerbliche Selbständige als auch Freiberufler.\n"
        "• <b>Stammkapital:</b> Mindestens 25.000 €. Gründung ist ab 12.500 € Einzahlung möglich (aber Nachschusspflicht im Haftungsfall auf den ausstehenden Teil).\n"
        "• <b>Sacheinlagen:</b> Möglich (z. B. Fahrzeuge, Patente; erfordert Wertgutachten).\n"
        "• <b>Haftung:</b> Beschränkt auf das Gesellschaftsvermögen.\n"
        "• <b>Buchführung:</b> Doppelte Buchführung.\n"
        "• <b>Handelsregister:</b> Ja.\n"
        "• <b>Gründungskosten:</b> Ca. 1.000 bis 3.000 € (Notar, Handelsregister)."
    ),
    (
        "Warum ist die Rechtsform der britischen Limited (Ltd.) in Deutschland nicht mehr relevant?",
        "Durch den <b>Brexit</b> droht privaten Inhabern einer Ltd. mit Verwaltungssitz in Deutschland die persönliche Haftung, da sie als Drittstaatengesellschaft eingestuft wird. Bestehende deutsche Ltds wurden/werden in UGs zwangsumgewandelt."
    ),
    (
        "Wie teilen sich Haftung und Mitspracherechte bei einer Kommanditgesellschaft (KG) auf?",
        "Eine KG unterscheidet zwei Gruppen von Gesellschaftern:\n"
        "1. <b>Komplementäre:</b> Haften unbeschränkt persönlich mit dem Privatvermögen und haben die Mitsprache- und Geschäftsführungsrechte.\n"
        "2. <b>Kommanditisten:</b> Haftung is auf ihre Einlage beschränkt; sie haben keine Mitspracherechte (daher ideal für passive Investoren).\n\n"
        "• <b>Buchführung:</b> Doppelte Buchführung.\n"
        "• <b>Handelsregister:</b> Ja."
    ),
    (
        "Was ist eine GmbH & Co. KG und welchen Vorteil bietet sie?",
        "<b>Definition:</b> Eine Kommanditgesellschaft (KG), bei der die Rolle des unbeschränkt haftenden Komplementärs nicht von einer natürlichen Person, sondern von einer GmbH übernommen wird.\n\n"
        "<b>Vorteil:</b> Dadurch wird das Haftungsrisiko des Komplementärs effektiv auf das Gesellschaftsvermögen der GmbH beschränkt, während gleichzeitig die steuerlichen Vorteile einer Personengesellschaft genutzt werden können."
    ),
    (
        "Was besagt die Kleinunternehmerregelung und wie hängt sie mit der Rechtsform zusammen?",
        "• <b>Bedeutung:</b> Befreiung von der Umsatzsteuerpflicht bei Umsätzen bis zu einer Grenze von 22 T€ (im Vorjahr) und voraussichtlich 50 T€ (im laufenden Jahr).\n"
        "• <b>Rechtsform:</b> Die Regelung hat <b>nichts</b> mit der Rechtsform zu tun. Man kann grundsätzlich in jeder Gesellschaftsform (auch als GmbH oder GbR) Kleinunternehmer sein, solange die Umsatzgrenzen eingehalten werden."
    ),
    (
        "Welche 8 Kriterien sind bei der Auswahl der Rechtsform zu berücksichtigen?",
        "1. <b>Anzahl der Gründer:</b> Alleine (Solo) oder im Team?\n"
        "2. <b>Haftungsbeschränkung:</b> Firmen- oder Privatvermögen?\n"
        "3. <b>Gründungskapital:</b> Mindestkapital ja oder nein?\n"
        "4. <b>Firmenname:</b> Phantasiename (Handelsregister) oder Geschäftsbezeichnung (Vor-/Nachname)?\n"
        "5. <b>Gründungskosten und -dauer</b>\n"
        "6. <b>Buchführung und Steuern:</b> EÜR vs. doppelte Buchführung, Gewerbesteuer-Freibetrag\n"
        "7. <b>Publizitätsvorschriften:</b> Veröffentlichungspflicht der Bilanzen?\n"
        "8. <b>Investorensuche und Mitbestimmung:</b> Wollen Investoren einsteigen?"
    ),
    (
        "Wie unterscheidet sich die Gründung von Personengesellschaften (Einzelunternehmen/GbR) von Kapitalgesellschaften (GmbH/UG)?",
        "• <b>Personengesellschaften (Einzelunternehmen/GbR):</b> Anmeldung beim Gewerbeamt geht sehr schnell (in wenigen Stunden, oft online), extrem geringe Kosten.\n"
        "• <b>Kapitalgesellschaften (GmbH/UG):</b> Aufwendig und teurer. Erfordert notarielle Beurkundung, Eröffnung eines Geschäftskontos, Einzahlung der Stammkapitaleinlagen, Nachweis an den Notar und Eintragung ins Handelsregister (Dauer: ca. 15 Werktage / mehrere Wochen; Kosten: > 1.000 €)."
    ),
    (
        "Welche steuerlichen Unterschiede (Gewerbesteuer, Verluste) bestehen zwischen Personen- und Kapitalgesellschaften?",
        "• <b>Gewerbesteuer-Freibetrag:</b>\n"
        "  - Kleingewerbetreibende und Personengesellschaften haben einen Freibetrag von <b>24.500 €</b>.\n"
        "  - Bei Kapitalgesellschaften gibt es <b>keinen</b> Freibetrag.\n"
        "• <b>Verlustverrechnung:</b>\n"
        "  - Verluste von Personengesellschaften können direkt mit anderen Einkünften der Gesellschafter verrechnet werden.\n"
        "  - Bei Kapitalgesellschaften werden Verlustvorträge gebildet, die nur mit zukünftigen Gewinnen der Gesellschaft verrechnet werden können."
    ),
    (
        "Welche Publizitätsvorschriften gelten für Kapitalgesellschaften und GmbH & Co. KGs?",
        "Sie unterliegen der <b>Veröffentlichungspflicht</b> ihrer Jahresabschlüsse/Bilanzen im Bundesanzeiger/Unternehmensregister.\n"
        "Zudem müssen Gesellschafter- und Geschäftsführerwechsel im Handelsregister veröffentlicht werden. Das macht sie für Wettbewerber transparenter als Personengesellschaften."
    ),
    (
        "Warum präferieren Investoren Kapitalgesellschaften (GmbH/UG) gegenüber Personengesellschaften?",
        "Investoren fordern eine strikte <b>Haftungsbegrenzung</b> auf das Gesellschaftsvermögen, um ihr privates Risiko zu minimieren.\n"
        "Zudem lassen sich Mitbestimmungsrechte im Gesellschaftsvertrag präzise regeln. So können Investoren z. B. eine <b>Sperrminorität (25,1 %)</b> erwerben, um wichtige strategische Entscheidungen (die eine 75 %-Mehrheit erfordern) zu blockieren."
    ),
    (
        "Skizziere den 6-Schritte-Ablaufplan zur Bargründung einer GmbH.",
        "1. <b>Notartermin:</b> Beurkundung des Gesellschaftsvertrags (Errichtungsprotokoll) -> Entstehung der Vor-GmbH (GmbH i. G.).\n"
        "2. <b>Kontoeröffnung:</b> Einrichtung des Geschäftskontos auf den Namen der GmbH i.G.\n"
        "3. <b>Kapitaleinzahlung:</b> Einzahlung der Stammeinlagen und Nachweis (Kontoauszug) an den Notar.\n"
        "4. <b>Einreichung:</b> Notar reicht Anmeldungsunterlagen elektronisch beim Handelsregister ein.\n"
        "5. <b>Gerichtskosten:</b> Bezahlung der Gerichtskostenrechnung durch die Gesellschaft.\n"
        "6. <b>Eintragung:</b> Eintragung im Handelsregister (ab hier existiert die GmbH wirksam)."
    ),
    (
        "Was ist eine Vor-GmbH (GmbH i.G.) und welches Haftungsrisiko besteht in dieser Phase?",
        "• <b>Bedeutung:</b> Entsteht durch die notarielle Beurkundung des Gesellschaftsvertrags vor der Eintragung im Handelsregister.\n"
        "• <b>Haftungsrisiko:</b> Bis zur endgültigen Eintragung besteht eine persönliche Haftung der Handelnden (Handelndenhaftung) und Vorbelastungshaftung der Gesellschafter.\n"
        "• <b>Empfehlung:</b> Die GmbH sollte vor dem Handelsregistereintrag nicht geschäftlich tätig werden, um diese private Haftung zu vermeiden."
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = '060_Gruendung_Rechtsformen.apkg'
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' mit {len(flashcards)} Karten.")
