import genanki
import os

MODEL_ID = 1829384800
DECK_ID = 2938475660

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
    'ITES Clean Model 010',
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
    'IT-Entrepreneurship :: 010 Studentische Selbstständigkeit'
)

flashcards = [
    # ===== 1. Die Rechnung =====
    (
        "Was ist eine Rechnung und in welcher Form muss sie erteilt werden? Ist eine Unterschrift zwingend nötig?",
        "• <b>Definition:</b> Eine gegliederte Aufstellung über die Forderung eines Entgelts für eine Leistung, die dem Empfänger die Nachprüfung ermöglichen muss.\n"
        "• <b>Form:</b> Muss in Textform erfolgen (schriftlich oder per E-Mail).\n"
        "• <b>Unterschrift:</b> Grundsätzlich **nicht erforderlich** (Ausnahme: Bei reinen Beratungsleistungen verlangen manche Gerichte eine Unterschrift, um die Erbringung abzusichern)."
    ),
    (
        "Welche 6 Pflichtangaben muss eine ordnungsgemäße Rechnung nach § 14 Abs. 4 UStG enthalten?",
        "1. Vollständiger Name und Anschrift des Leistenden und des Empfängers.\n"
        "2. Steuernummer oder die 11-stellige Steuer-Identifikationsnummer (Steuer-ID) des Leistenden.\n"
        "3. Ausstellungsdatum und eine fortlaufende Rechnungsnummer.\n"
        "4. Art und Umfang der Leistung sowie der Leistungszeitpunkt.\n"
        "5. Anzuwendender Steuersatz sowie Steuer- und Rechnungsbetrag (Netto, MwSt, Brutto).\n"
        "6. Ggf. Hinweis auf den Grund einer Steuerbefreiung (z. B. Kleinunternehmerregelung)."
    ),

    # ===== 2. Kleinunternehmerregelung =====
    (
        "Was ist die Kleinunternehmerregelung (§ 19 UStG) und welche Umsatzgrenzen gelten ab 01.01.2025?",
        "Eine Vereinfachungsregelung, die Unternehmern mit niedrigen Umsätzen das Wahlrecht gewährt, keine Umsatzsteuer in Rechnungen auszuweisen (weitgehend wie Nichtunternehmer behandelt zu werden).\n"
        "• <b>Grenze Vorjahr:</b> &lt; **25.000 €** (netto, vor 2025: 22.000 €).\n"
        "• <b>Grenze laufendes Jahr:</b> &lt; **100.000 €** (brutto/netto, harte Grenze ab 2025)."
    ),
    (
        "Was gilt bezüglich der Kleinunternehmer-Umsatzgrenzen im Jahr der Unternehmensgründung?",
        "• <b>Hochrechnung:</b> Wird die Tätigkeit unterjährig begonnen, muss der erzielte Umsatz auf einen Jahres-Gesamtumsatz hochgerechnet werden.\n"
        "• <b>Überschreitung:</b> Erreicht der hochgerechnete Umsatz voraussichtlich mehr als 25.000 €, entfällt der Kleinunternehmerstatus sofort.\n"
        "• <b>Harte Grenze:</b> Wird die 100.000 €-Grenze im laufenden Jahr überschritten, ist ab der reißenden Rechnung sofort zur Regelbesteuerung zu wechseln."
    ),
    (
        "Welche EU-weiten Änderungen gelten seit 1.1.2025 für Kleinunternehmer?",
        "• <b>Öffnung:</b> Die Kleinunternehmerregelung kann nun auch von deutschen Selbstständigen in anderen EU-Ländern und umgekehrt genutzt werden.\n"
        "• <b>Voraussetzung:</b> Gültige Umsatzsteuer-Identifikationsnummer (USt-IdNr.) und der EU-weite Gesamtumsatz des Unternehmens bleibt unter **100.000 €**."
    ),
    (
        "Welche Pflichten und Fristen gelten für Kleinunternehmer bezüglich der E-Rechnung im B2B-Bereich?",
        "• <b>Ab 01.01.2025:</b> Alle Unternehmen müssen im B2B-Bereich elektronische Rechnungen (E-Rechnungen) empfangen und verarbeiten können (gilt auch für Kleinunternehmer).\n"
        "• <b>Ab 01.01.2028:</b> Pflicht zur Erstellung und zum Versand von E-Rechnungen im B2B. Einfache PDFs oder Papierrechnungen sind dann im B2B-Bereich unzulässig (Ausnahme: Kleinbetragsrechnungen bis 250 € oder bestimmte freigestellte Gruppen)."
    ),
    (
        "Was bedeutet die Inanspruchnahme der Kleinunternehmerregelung für Rechnungen und Steuererklärungen konkret?",
        "1. Rechnungen müssen korrekt sein, weisen aber nur den Nettobetrag aus (= Endbetrag).\n"
        "2. Es darf **keine Umsatzsteuer** ausgewiesen werden.\n"
        "3. Es müssen keine Umsatzsteuervoranmeldungen abgegeben werden.\n"
        "4. Jede Rechnung muss einen Hinweis auf die Befreiung enthalten: <i>„Umsatzsteuerfrei gemäß § 19 UStG (Kleinunternehmerregelung)“</i>."
    ),
    (
        "Unter welchen Umständen lohnt es sich, freiwillig auf die Kleinunternehmerregelung zu verzichten (Optieren), und welche Frist bindet den Gründer?",
        "• <b>Wann sinnvoll:</b> Wenn zu Beginn hohe Investitionen mit ausweisbarer Umsatzsteuer anstehen (z. B. Kauf von teurer Hardware für 4.000 €), um sich die gezahlte Umsatzsteuer als Vorsteuer vom Finanzamt erstatten zu lassen.\n"
        "• <b>Bindungsfrist:</b> Die Option zur Regelbesteuerung bindet den Unternehmer für **5 Kalenderjahre**."
    ),

    # ===== 3. Freiberufler vs. Gewerbe =====
    (
        "Welche Nachteile hat die Anmeldung eines Gewerbes für einen studentischen Entwickler?",
        "• Zahlung von Gewerbesteuer (bei Überschreitung des Freibetrags von 24.500 €).\n"
        "• Pflichtmitgliedschaft in der IHK (Mindestbeiträge fallen an) und Berufsgenossenschaft.\n"
        "• Doppelte Buchführung und Bilanzierungspflicht, wenn der Gewinn &gt; 60.000 € oder Umsatz &gt; 600.000 € liegt (erhöhte Steuerberaterkosten).\n"
        "• Soll-Versteuerung statt Ist-Versteuerung ab bestimmten Grenzen."
    ),
    (
        "Welche steuerlichen Vorteile bietet der Status des Freiberuflers?",
        "• Keine Gewerbesteuer.\n"
        "• Keine Gewerbeanmeldung erforderlich.\n"
        "• Keine doppelte Buchführung; eine einfache Einnahmen-Überschuss-Rechnung (EÜR) reicht aus.\n"
        "• Unabhängig von Umsatzgrenzen gilt die Ist-Versteuerung auf Antrag (Umsatzsteuer wird erst fällig, wenn der Kunde tatsächlich bezahlt hat)."
    ),
    (
        "Wann gilt ein Softwareentwickler steuerrechtlich als Freiberufler (ähnlicher Beruf)?",
        "1. <b>Qualifikation:</b> Nachweis der Tiefe und Breite des Wissens eines Diplom- bzw. Master-Informatikers. Bei Masterstudenten durch den Bachelorabschluss gegeben; bei Bachelorstudenten in den ersten Semestern meist nicht (Graubereich in den letzten Semestern).\n"
        "2. <b>Tätigkeit:</b> Es darf keine **Trivialsoftware** entwickelt werden. Es muss eine ingenieurmäßige Vorgehensweise vorliegen (Planung, Konstruktion, Überwachung)."
    ),
    (
        "Was versteht die Rechtsprechung unter steuerlicher „Trivialsoftware“?",
        "Standardisierte Programme zur reinen Datenabspeicherung, die für jedermann zugänglich sind (z. B. Telefonbücher, Lexika, einfache Vokabeltrainer).\n"
        "Die Entwicklung von Individualsoftware anhand klassischer ingenieurmäßiger Vorgehensweise gilt **nicht** als Trivialsoftware."
    ),
    (
        "Ist der Verkauf von selbsterstellten Softwarelizenzen eine freiberufliche Tätigkeit?",
        "<b>Nein, das ist gewerblicher Handel.</b>\n"
        "Der Vertrieb, Verkauf oder die Vermietung von Softwarelizenzen (auch von selbst geschriebener Software) wird steuerlich stets als gewerbliche Tätigkeit eingestuft."
    ),
    (
        "Was versteht man unter einer „gemischten Tätigkeit“ (z. B. Softwareentwicklung + Hardwareverkauf) und wie wird sie steuerlich behandelt?",
        "• <b>Definition:</b> Wenn sowohl freiberufliche (Entwicklung) als auch gewerbliche (Handel mit Hardware) Leistungen erbracht werden.\n"
        "• <b>Behandlung:</b> Wenn die Tätigkeiten untrennbar sind, entscheidet das Gesamtbild der Gesamtleistung. Überwiegt die Softwareentwicklung und prägt sie das Bild, kann die Tätigkeit insgesamt als freiberuflich eingestuft werden.\n"
        "• <b>Sicherer Weg:</b> Hardware im Namen und Auftrag des Kunden beschaffen (Kunde zahlt direkt an Händler)."
    ),
    (
        "Wie kann ein fälschlicherweise angemeldetes Gewerbe korrigiert werden, wenn tatsächlich eine freiberufliche Tätigkeit vorliegt?",
        "Das Gewerbe darf nicht nur abgemeldet werden (da das Finanzamt sonst für den Zeitraum gewerbliche Einkünfte vermutet). Es muss eine **rückwirkende Abmeldung des Gewerbes** wegen falscher Deklaration beim Gewerbeamt eingereicht werden."
    ),

    # ===== 4. Auslandsgeschäfte =====
    (
        "Wie wird die Bereitstellung von Software per Download an ein B2B-Unternehmen in der Schweiz oder im UK umsatzsteuerlich behandelt?",
        "• <b>Klassifizierung:</b> Durch den Download handelt es sich um eine „sonstige Leistung“ (nicht verkörperte Software).\n"
        "• <b>Leistungsort:</b> Liegt am Sitz des Empfängers (Schweiz/UK).\n"
        "• <b>Rechnung:</b> Wird netto (ohne Umsatzsteuer) ausgestellt.\n"
        "• <b>Pflichtangaben:</b> Eigene USt-IdNr., USt-IdNr. des Abnehmers sowie der Vermerk **„Reverse Charge Verfahren“** (Steuerschuldnerschaft des Empfängers)."
    ),
    (
        "Unter welchen Bedingungen müssen Zahlungen aus dem Ausland bei der Bundesbank gemeldet werden?",
        "Zahlungseingänge aus dem Ausland müssen ab einem Betrag von **50.000 €** (ab 2025, davor 12.500 €) gemäß Außenwirtschaftsverordnung gemeldet werden."
    ),
    (
        "Gilt bei Auslands-Downloads von Software ein Zoll?",
        "<b>Nein.</b> Wegen des reinen Downloads (kein physischer Datenträger) fällt kein Zoll an und es ist keine Zollanmeldung erforderlich.\n"
        "<i>Ausnahme:</i> Ausfuhrgenehmigungen können bei Software mit starker Verschlüsselungstechnologie relevant sein."
    ),

    # ===== 5. Einkommensteuer, Abschreibungen, Verlustvortrag =====
    (
        "Was ist eine Einnahmen-Überschuss-Rechnung (EÜR)?",
        "Eine einfache Gewinnermittlungsmethode nach § 4 Abs. 3 EStG. Dabei werden die tatsächlichen (Netto-)Betriebseinnahmen eines Kalenderjahres den tatsächlichen (Netto-)Betriebsausgaben gegenübergestellt.\n"
        "Es gilt das Zufluss-Abfluss-Prinzip."
    ),
    (
        "Was ist ein „vortragsfähiger Verlust“ (z. B. durch Anschaffung einer Workstation für 3.900 € im Studium ohne Einnahmen)?",
        "• <b>Konzept:</b> Übersteigen die abzugsfähigen Ausgaben (z. B. durch Abschreibung der Hardware) im Steuerjahr die Einnahmen, entsteht ein steuerlicher Verlust.\n"
        "• <b>Verlustvortrag:</b> Dieser negative Betrag wird vom Finanzamt festgestellt und in zukünftige Jahre vorgetragen, um ihn dort vom zu versteuernden Einkommen abzuziehen."
    ),
    (
        "Wann lohnt sich ein steuerlicher Verlustvortrag aus der Studienzeit?",
        "Er lohnt sich nur dann, wenn das zu versteuernde Einkommen im Folgejahr (Berufseinstieg) über dem **Existenzminimum (Grundfreibetrag)** liegt, da unterhalb des Freibetrags ohnehin keine Einkommensteuer gezahlt werden muss."
    ),
    (
        "Sind Masterstudienkosten als Werbungskosten absetzbar?",
        "• <b>Ja, als Werbungskosten:</b> Ein Masterstudium gilt steuerlich als Weiterbildung, da mit dem Bachelor die erste berufsqualifizierende Ausbildung abgeschlossen wurde. Verluste können unbegrenzt vorgetragen werden.\n"
        "• <b>Ausnahme (konsekutives Masterstudium):</b> Steht der Master in engem sachlichen und zeitlichen Zusammenhang zum Bachelor und stellt er einen einheitlichen Ausbildungsgang dar (z. B. Staatsexamen), kann er als Teil der Erstausbildung gewertet werden (dann nur eingeschränkt als Sonderausgaben absetzbar)."
    ),
    (
        "Wie unterscheidet sich die Übungsleiterpauschale von der Ehrenamtspauschale?",
        "• <b>Übungsleiterpauschale (§ 3 Nr. 26 EStG):</b> Bis zu **3.000 € p.a.** steuerfrei für nebenberufliche pädagogische, künstlerische oder lehrende Tätigkeiten (z. B. Dozenten an Hochschulen).\n"
        "• <b>Ehrenamtspauschale (§ 3 Nr. 26a EStG):</b> Bis zu **840 € p.a.** steuerfrei für sonstige ehrenamtliche Tätigkeiten im gemeinnützigen Bereich."
    ),
    (
        "Darf die Übungsleiterpauschale mit dem steuerlichen Grundfreibetrag addiert werden?",
        "<b>Ja.</b> Die Freibeträge zählen zusammen.\n"
        "Verdient man z. B. 8.100 € aus einer Werkstudentenstelle und zusätzlich 3.000 € über die Übungsleiterpauschale, bleibt das gesamte Einkommen steuerfrei, da die Pauschale steuerfrei gestellt ist und das Werkstudentengehalt unter dem Grundfreibetrag liegt."
    ),
    (
        "Wie wirkt sich eine steuerfreie Einnahme (z. B. Dozententätigkeit) auf den Vorsteuerabzug eines Selbstständigen aus?",
        "Da die Einnahmen aus der Dozententätigkeit umsatzsteuerfrei sind, darf die Vorsteuer aus Eingangsrechnungen (z. B. Kauf eines Laptops) nur **anteilig** abgezogen werden.\n"
        "Sind z. B. 10% der Gesamteinnahmen steuerfrei, können nur 90% der gezahlten Vorsteuer geltend gemacht werden."
    ),

    # ===== 6. Krankenversicherung & Arbeitszeit =====
    (
        "Welche wöchentliche Arbeitszeitgrenze gilt für Werkstudenten während der Vorlesungszeit und warum?",
        "Maximal **20 Wochenstunden**.\n"
        "Wird diese Grenze während des Semesters überschritten, verliert der Student das studentische Privileg in der Sozialversicherung und muss sich regulär als Arbeitnehmer kranken- und pflegeversichern (während der Semesterferien darf voll gearbeitet werden)."
    ),
    (
        "Bis zu welchem Einkommen ist eine beitragsfreie Familienversicherung für Studenten (unter 25 Jahren) möglich?",
        "Das regelmäßige monatliche Einkommen darf die gesetzliche Grenze nicht überschreiten (2024: **505 € mtl.** plus anteilige Werbungskostenpauschale). Liegt der Verdienst darüber, muss eine eigene studentische Krankenversicherung abgeschlossen werden."
    ),

    # ===== 7. Das Angebot (Struktur & Inhalt) =====
    (
        "Was ist ein Angebot im rechtlichen Sinn (BGB)?",
        "Eine empfangsbedürftige Willenserklärung, die auf einen Vertragsschluss gerichtet ist und so bestimmt sein muss, dass sie durch ein einfaches, deckungsgleiches „Ja“ (Annahme) des Empfängers angenommen werden kann."
    ),
    (
        "Nenne 6 wichtige Bausteine, die in einem professionellen Software-Entwicklungsangebot enthalten sein sollten.",
        "1. Auftragsgegenstand / Aufgabenstellung.\n"
        "2. Vorgehensweise und detaillierte Aufwandskalkulation (Stunden/Tage).\n"
        "3. Zeitplan der Auslieferung und Test-Fristen für den Kunden.\n"
        "4. Mitwirkungspflichten des Auftraggebers (Testdaten, Zugänge, Lizenzen).\n"
        "5. Regelungen zu den Rechten am Quellcode.\n"
        "6. Haftungsbegrenzung (angepasst an IT-Haftpflicht).\n"
        "7. Support- und Nachbesserungsregelungen."
    ),
    (
        "Warum ist die genaue Definition von Kunden-Mitwirkungspflichten im Angebot wichtig?",
        "Damit Verzögerungen, die durch den Kunden entstehen (z. B. verspätete Bereitstellung von Testdaten, Schnittstellen oder Systemzugängen), nicht dem Entwickler angelastet werden können (Vermeidung von Verzug und Schadensersatzansprüchen)."
    ),
    (
        "Wie verhält es sich mit der Haftung in Allgemeinen Geschäftsbedingungen (AGB) von IT-Dienstleistern?",
        "Die Haftung kann in AGB **eingeschränkt, aber nicht komplett ausgeschlossen** werden. Ein vollständiger Ausschluss macht die gesamte AGB-Klausel unwirksam.\n"
        "Die Haftungsgrenzen sollten unbedingt mit der eigenen IT-Haftpflichtversicherung abgestimmt werden."
    ),
    (
        "Welche Preisgestaltungsmodelle gibt es bei Softwareprojekten und wo liegen die Vorteile?",
        "• <b>Festpreismodell:</b> Fester Preis unabhängig vom Aufwand (Vorteil Kunde: Budgetsicherheit).\n"
        "• <b>Festpreis mit Öffnungsklausel:</b> Festpreis mit Toleranzgrenze (z. B. +10% bei Mehraufwand nach Absprache).\n"
        "• <b>Abrechnung nach Aufwand (Time & Material):</b> Abrechnung geleisteter Stunden (Vorteil Entwickler: kein Risiko bei unklaren Anforderungen / Moving Targets).\n"
        "• <b>Abrechnung nach Aufwand mit Deckelung (Cap).</b>"
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = '010_Studentische_Selbststaendigkeit.apkg'

# Save to root directory
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' im Hauptverzeichnis.")

# Also save to Managementkompetenz/itEnterpreneurship/Folien/
folien_dir = os.path.join('Managementkompetenz', 'itEnterpreneurship', 'Folien')
if os.path.exists(folien_dir):
    dest_path = os.path.join(folien_dir, output_filename)
    genanki.Package(anki_deck).write_to_file(dest_path)
    print(f"Erfolgreich kopiert nach: '{dest_path}'.")
