import genanki
import os

MODEL_ID = 1829384795
DECK_ID = 2938475655

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
    'ITES Clean Model 015',
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
    'IT-Entrepreneurship :: 015 IT-Recht für Softwareentwickler'
)

flashcards = [
    # ===== 1. Verträge für Softwareprojekte =====
    (
        "Gibt es im deutschen Recht einen gesetzlichen Vertragstyp „Software-Entwicklung“? Wie lösen Gerichte dies?",
        "<b>Nein, einen solchen Vertragstyp gibt es nicht.</b>\n"
        "Gerichte ordnen Softwareentwicklungs-Projekte so weit wie möglich einem der bestehenden gesetzlichen Vertragstypen des BGB zu:\n"
        "• <b>Werkvertrag</b>\n"
        "• <b>Werklieferungsvertrag</b>\n"
        "• <b>Dienstvertrag</b>\n"
        "Maßgeblich für die Einordnung ist der ursprüngliche Wille der Parteien bei Vertragsabschluss, nicht die Überschrift des Dokuments."
    ),
    (
        "Was ist der Zweck eines Softwareentwicklungs-Vertrages?",
        "• <b>Gemeinsames Verständnis:</b> Festlegung des zu erbringenden Leistungsumfangs (Funktionen) sowie expliziter Ausschluss nicht zu erbringender Leistungen.\n"
        "• <b>Vorabregelungen für Konflikte:</b> Regelungen zur Abnahme, Mängelbeseitigung, Beendigung der Zusammenarbeit und Haftungsbeschränkungen.\n"
        "• <b>Vermeidung von kostenloser Vorleistung:</b> Klärung, ab wann Entwicklungsaufwände vergütet werden."
    ),
    (
        "Nenne die Hauptmerkmale eines Werkvertrags (§ 631 BGB) bei der Softwareentwicklung.",
        "• <b>Erfolgsbezug:</b> Der Entwickler schuldet die Herstellung eines Werks, also das Erreichen eines konkreten, fehlerfreien Erfolgs.\n"
        "• <b>Fälligkeit:</b> Der gesamte Werklohn wird erst nach der Abnahme (ohne wesentliche Mängel) fällig.\n"
        "• <b>Rücktritt:</b> Der Auftraggeber kann bis zur Fertigstellung jederzeit zurücktreten (§ 649 BGB). Er muss dann den vollen Werklohn abzüglich ersparter Aufwendungen zahlen."
    ),
    (
        "Was versteht man unter einem Werklieferungsvertrag (§ 651 BGB) im Kontext von Software?",
        "• <b>Gegenstand:</b> Die Lieferung einer zu erzeugenden beweglichen Sache. Die Rechtsprechung stuft Individualsoftware meist als eine solche bewegliche Sache ein.\n"
        "• <b>Rechtsfolge:</b> Es gelten die Vorschriften des **Kaufrechts**.\n"
        "• <b>Fälligkeit & Gewährleistung:</b> Kaufpreis ist bei Ablieferung fällig (nicht erst bei Abnahme). Die 2-jährige Gewährleistung (unentgeltliche Mängelbeseitigung) beginnt ebenfalls mit der Ablieferung."
    ),
    (
        "Nenne die Hauptmerkmale eines Dienstvertrags (§ 611 BGB) bei der Softwareentwicklung.",
        "• <b>Tätigkeitsbezug:</b> Der Entwickler schuldet das Bemühen (ordentliche Programmierarbeit gemäß 'Stand der Technik' / 'best practice'), garantiert aber kein bestimmtes Erfolgsergebnis.\n"
        "• <b>Vergütung:</b> Wird meist nach Zeitaufwand (Stunden- oder Tagessätze) bezahlt.\n"
        "• <b>Fälligkeit & Gewährleistung:</b> Mit Zahlung gilt die Leistung als abgenommen; es gibt im gesetzlichen Regelfall keine unbezahlte Pflicht zur Nachbesserung."
    ),
    (
        "Welcher Vertragstyp ist aus Sicht des Softwareentwicklers der risikoärmste und warum?",
        "Der **Dienstvertrag**.\n"
        "Da der Entwickler kein konkretes, fehlerfreies Endergebnis garantiert, sondern nur ordentliche Programmierarbeit (Bemühen) schuldet. Zudem entfällt die unentgeltliche Mängelbeseitigungspflicht nach der Bezahlung."
    ),
    (
        "Wie kann ein Dienstvertrag im Angebot oder Vertrag so gestaltet werden, dass Bug-Fixing vergütet wird?",
        "Durch das Einfügen eines expliziten Absatzes, z. B.:\n"
        "<i>„Werden vom Auftraggeber bei der vertraglich festgelegten Leistung berechtigt Mängel beanstandet, so ist der Auftragnehmer zur Nachbesserung verpflichtet und berechtigt. Solche Leistungen werden gemäß den vereinbarten Honorarsätzen abgerechnet.“</i>\n"
        "Zudem sollte formuliert werden: <i>„Mit Zahlung der Rechnung gilt die Leistung als abgenommen.“</i>"
    ),
    (
        "Erkläre den typischen Studentenfall zur Vertragseinordnung (Studentin arbeitet über 1 Jahr für Entgelt, will in Prüfungszeit pausieren, wird wegen Fehlerbehebung verklagt).",
        "• <b>Problem:</b> Die Studentin arbeitete ohne schriftliche Klärung des Vertragstyps. Sie ging von einem Dienstvertrag (Tätigkeit gegen Entgelt) aus.\n"
        "• <b>Rechtliche Einordnung:</b> Das Gericht stufte es als **Werkvertrag** (Schuld eines Gesamterfolgs) ein. Da ein Werkvertrag den Erfolg schuldet, war die Studentin trotz Prüfungsvorbereitung gesetzlich verpflichtet, die Mängel am geschuldeten Werk unentgeltlich zu beseitigen. Sie verlor den Prozess."
    ),

    # ===== 2. Softwarevermarktung =====
    (
        "Welche rechtlichen Grundlagen gelten für die verschiedenen Software-Verwertungsarten (Verkauf, Vermietung, Open Source)?",
        "<table>"
        "<tr><th>Verwertungsart</th><th>Rechtliche Grundlage</th></tr>"
        "<tr><td><b>Verkauf von SW-Kopien (auf Dauer)</b></td><td>Kaufvertrag BGB (Gewährleistung, Erschöpfungsgrundsatz)</td></tr>"
        "<tr><td><b>Vermietung / SaaS / ASP (zeitlich befristet)</b></td><td>Mietvertrag BGB (Haftung für Mängelfreiheit über gesamte Dauer)</td></tr>"
        "<tr><td><b>Open Source</b></td><td>Urheberrecht + Schenkungsrecht (Haftungsbeschränkung auf Arglist)</td></tr>"
        "</table>"
    ),
    (
        "Welche rechtlichen Folgen hat der Verkauf einer Softwarekopie zur dauerhaften Nutzung (Kaufvertrag)?",
        "• <b>Gewährleistung:</b> 2 Jahre Haftung ab Lieferung für Mängelfreiheit.\n"
        "• <b>Weiterverkauf:</b> Der Käufer darf seine gebrauchte Softwarelizenz an Dritte weiterverkaufen (Erschöpfungsgrundsatz, siehe UsedSoft-Urteil des EuGH).\n"
        "• <b>Verbraucherschutz:</b> Im B2C-Bereich gelten erhöhte Verbraucherrechte."
    ),
    (
        "Welche rechtlichen Folgen hat die Vermietung von Software (SaaS / ASP / Cloud Computing)?",
        "• <b>Vertragsnatur:</b> Es handelt sich um einen **Mietvertrag** (nicht Dienstvertrag, da ein Bereitstellungserfolg geschuldet wird).\n"
        "• <b>Haftung:</b> Der Vermieter haftet während der gesamten Mietzeit für die Mängelfreiheit (erfordert Support/Wartung).\n"
        "• <b>Weiterverkauf:</b> Weitervermietung und Weiterverkauf durch den Kunden können wirksam ausgeschlossen werden.\n"
        "• <b>Wirtschaftlich:</b> Laufende Mieteinnahmen erhöhen den Unternehmenswert."
    ),
    (
        "Was unterscheidet den Fremdvertrieb über Lizenzen vom reinen Verkauf der Software?",
        "• <b>Rechtsnatur:</b> Es gibt keine speziellen gesetzlichen Regelungen für Software-Lizenzverträge; alle Rechte und Pflichten müssen im Vertrag vereinbart werden.\n"
        "• <b>Inhalt:</b> Regelungen über erlaubte Kopienanzahl, Vertriebswege, Bugfixing und Erweiterungen.\n"
        "• <b>Haftung:</b> Die Haftung des Lizenzgebers kann vertraglich weitestgehend ausgeschlossen werden."
    ),
    (
        "Was ist der Unterschied zwischen Leasing und Miete bei Software?",
        "• <b>Miete:</b> Reine zeitlich begrenzte Nutzung der Software.\n"
        "• <b>Leasing:</b> Ähnelt der Miete, beinhaltet aber meist ein Kaufrecht oder einen automatischen Eigentumsübergang nach Ende der Laufzeit. Leasingraten werden steuerlich direkt als Kosten (beim Leasingnehmer) bzw. Umsatz (beim Leasinggeber) gebucht."
    ),
    (
        "Was ist ein „Mietkauf“ bei Software?",
        "Ein Mietvertrag, bei dem dem Mieter das Recht eingeräumt wird, die Software innerhalb einer Frist käuflich zu erwerben.\n"
        "Bereits gezahlte Mieten werden auf den Kaufpreis angerechnet, oder das Eigentum geht mit Zahlung der letzten Rate automatisch über. Dient oft als Finanzierungshilfe für Neugründungen."
    ),

    # ===== 3. Spenden, Schenkungen, Crowdfunding =====
    (
        "Gilt die Bereitstellung eines Open-Source-Programms gegen eine freiwillige Spende nach deutschem Recht als steuerliche Spende?",
        "<b>Nein.</b>\n"
        "Eine steuerlich absetzbare Spende (§ 10b EStG) setzt voraus:\n"
        "1. Förderung gemeinnütziger, mildtätiger oder kirchlicher Zwecke.\n"
        "2. Keine Gegenleistung (uneigennützig).\n"
        "Da der Erhalt der Software als Gegenleistung steht, handelt es sich steuerlich um eine **normale Einnahme** beim Entwickler und eine Betriebsausgabe/Kosten beim Zahler."
    ),
    (
        "Kann ein selbständiger Softwareentwickler steuerliche Gemeinnützigkeit beanspruchen?",
        "<b>Nein.</b>\n"
        "Gemeinnützigkeit (§ 51 AO) ist nach deutschem Recht ausschließlich **Körperschaften** (z. B. Vereinen, gGmbHs) vorbehalten. Einzelunternehmen oder Personengesellschaften können nicht gemeinnützig sein."
    ),
    (
        "Wie wird Crowdfunding steuerlich behandelt?",
        "Da beim Crowdfunding in der Regel eine Gegenleistung versprochen wird (z. B. spätere Nutzungslizenz oder Produktlieferung), handelt es sich um eine steuerpflichtige **Einnahme** beim Zahlungsempfänger und nicht um eine steuerfreie Spende."
    ),
    (
        "Kann man Zahlungen für Software als Schenkung deklarieren, um Steuern zu sparen? Welche Gefahr besteht (Fall Michael Ballweg)?",
        "• <b>Schenkung:</b> Schenkungen an Privatpersonen sind bis zu 20.000 € steuerfrei.\n"
        "• <b>Gefahr:</b> Wird das Geld zweckgebunden gesammelt und dann anderweitig verwendet, drohen Anklagen wegen Betrugs, Unterschlagung oder Steuerhinterziehung (wie im Prozess gegen den „Querdenken“-Gründer Michael Ballweg, der jedoch letztlich weitestgehend freigesprochen wurde)."
    ),

    # ===== 4. Software schützen (Urheberrecht vs. Patentrecht) =====
    (
        "Vergleiche Urheberrecht und Patentrecht bezüglich Schutzgegenstand, Kosten und Laufzeit bei Software.",
        "<table>"
        "<tr><th>Merkmal</th><th>Urheberrecht</th><th>Patentrecht</th></tr>"
        "<tr><td><b>Schutzgegenstand</b></td><td>Der konkrete Code (äußere Form, Schutz vor 1:1-Kopien)</td><td>Die allgemeine technische Lösung eines Problems (unabhängig vom Code)</td></tr>"
        "<tr><td><b>Kosten</b></td><td>Kostenlos (entsteht automatisch bei Schöpfung)</td><td>Teuer (ca. 1.500–4.000 € Anmeldung + Ländergebühren)</td></tr>"
        "<tr><td><b>Schutzdauer</b></td><td>70 Jahre nach dem Tod des Urhebers</td><td>Maximal 20 Jahre ab Anmeldung</td></tr>"
        "</table>"
    ),
    (
        "Welche Anforderungen müssen für den Urheberschutz einer Software erfüllt sein?",
        "1. <b>Schöpfung:</b> Gestalterische, menschliche Tätigkeit (subjektive Neuheit).\n"
        "2. <b>Geistiger Gehalt:</b> Einsatz nicht rein mechanischer/automatisierter Mittel.\n"
        "3. <b>Äußere Form-Wahrnehmbarkeit:</b> Keine bloß abstrakte Idee.\n"
        "4. <b>Individualität / Schöpfungshöhe.</b>"
    ),
    (
        "Genießen KI-generierte Codes (z. B. von ChatGPT) Urheberschutz nach deutschem Recht?",
        "<b>Nein.</b>\n"
        "Nach § 2 UrhG gelten nur „persönliche geistige Schöpfungen“ als geschützte Werke. Das setzt zwingend eine **menschliche Schöpfung** voraus. Von Algorithmen oder Maschinen erzeugte Texte und Programmcodes sind gemeinfrei."
    ),
    (
        "Wem gehören Urheberrechte an Software, die im Rahmen einer Abschlussarbeit (Thesis) an einer Hochschule erstellt wurde?",
        "• <b>Grundsatz:</b> Das Urheberrecht sowie die Nutzungs- und Verwertungsrechte liegen beim **Studenten** (Thesis-Verfasser). Das Urheberrecht ist unverzichtbar.\n"
        "• <b>Hochschule:</b> Hat nur Anspruch auf das physikalische/digitale Original der Arbeit zur Archivierung als Prüfungsleistung.\n"
        "• <b>Abtretung:</b> Über Thesis-Verträge (z. B. mit Unternehmen) können dem Partner jedoch exklusive Nutzungsrechte eingeräumt werden."
    ),
    (
        "Wem gehören Urheberrechte an Software, die von Werkstudenten oder angestellten Entwicklern geschrieben wird?",
        "Dem **Arbeitgeber**.\n"
        "Nach § 69b UrhG (und dem Arbeitnehmererfindungsgesetz ArbEG) stehen dem Arbeitgeber alle exklusiven Nutzungs- und Verwertungsrechte für Software zu, die in Erfüllung von Dienstpflichten erstellt wurde."
    ),
    (
        "Ist „Reverse Engineering“ (Rückbau und Testen) bei Software gesetzlich erlaubt?",
        "• <b>Geschäftsgeheimnisgesetz (§ 3 GeschGehG):</b> Erlaubt grundsätzlich das Beobachten, Testen und Rückbauen rechtmäßig veröffentlichter Produkte.\n"
        "• <b>Aber (Urheberrechtsschutz):</b> Bei Software greift zusätzlich das Urheberrecht. Ein Rückübersetzen (Dekompilieren) des Binärcodes in Quellcode unterliegt extrem strengen Hürden (z. B. nur zur Herstellung von Interoperabilität).\n"
        "• <b>Ergebnis:</b> Reverse Engineering von Software ist im Regelfall **nicht zulässig**."
    ),

    # ===== 5. Softwarepatente & Standards =====
    (
        "Unter welchen Voraussetzungen ist eine Softwareerfindung patentfähig (Technizität)?",
        "Die Software darf nicht nur „als solche“ Programme für Datenverarbeitungsanlagen darstellen (§ 1 Abs. 3 PatG). Sie muss **Technizität** aufweisen:\n"
        "Sie muss ein **konkretes technisches Problem außerhalb der Hardware** lösen (computerimplementierte Erfindung).\n"
        "• <i>Positivbeispiel:</i> Motorsteuerung beim Kfz, Bremsensteuerung, RSA-Schlüsselpaarbestimmung.\n"
        "• <i>Negativbeispiel:</i> Rechtschreibprüfung (Unterstreichen falscher Wörter)."
    ),
    (
        "Was bedeutet die Anforderung der „Neuheit“ beim Patentrecht für Entwickler und Forscher?",
        "Eine Lösung gilt nur als neu, wenn sie vor dem Tag der Anmeldung **nirgends auf der Welt** veröffentlicht wurde.\n"
        "Eine eigene wissenschaftliche Publikation vor der Anmeldung ist bereits **patentschädlich**. Es gilt die Regel: **Erst patentieren, dann publizieren!**"
    ),
    (
        "Was versteht man unter einem „wesentlichen Patent“ in Technologiestandards und wie wird ein Monopol verhindert (FRAND)?",
        "• <b>Wesentliches Patent (SEP):</b> Ein Patent, das so tief in einem Standard (z. B. LTE, MP3) verankert ist, dass jede Implementierung des Standards zwangsläufig das Patent verletzt.\n"
        "• <b>FRAND-Bedingungen:</b> Der Patentinhaber muss sich verpflichten, Lizenzen fair, angemessen und nicht-diskriminierend anzubieten:\n"
        "  - <b>F (Fair):</b> Keine Bündelung (no bundling) mit standardfremden Patenten.\n"
        "  - <b>R (Reasonable):</b> Angemessene Lizenzraten (oft Umsatzprozent).\n"
        "  - <b>AND (Non-Discriminatory):</b> Gleichbehandlung aller Lizenznehmer."
    ),
    (
        "Was ist ein „Patent-Troll“ und wie gehen diese gegen App-Entwickler vor (Fall Uniloc)?",
        "• <b>Patent-Troll:</b> Eine Firma, die selbst keine Produkte herstellt, sondern Patente aufkauft, um Lizenzgebühren einzuklagen.\n"
        "• <b>Vorgehensweise:</b> Statt finanzstarke Plattformen (z. B. Google selbst) zu verklagen, verklagen sie kleine App-Entwickler, die Apps im Play Store anbieten. Da sich die Entwickler die teuren US-Gerichtskosten nicht leisten können, erzwingen die Trolle Vergleiche (oft zehntausende Euro)."
    ),
    (
        "Wie ist das Prozesskostenrisiko bei Patentklagen in den USA im Vergleich zu Deutschland?",
        "• <b>USA:</b> Jede Partei trägt ihre eigenen Anwaltskosten selbst (unabhängig davon, wer gewinnt). Daher ist das finanzielle Risiko für Kläger gering und die Hemmschwelle für Missbrauch niedrig.\n"
        "• <b>Deutschland:</b> Der Verlierer eines Prozesses muss auch die gesetzlichen Anwalts- und Gerichtskosten des Gewinners tragen, was Trolle abschreckt."
    ),
    (
        "Besteht für einen deutschen App-Entwickler ein reales Risiko, in den USA wegen Patentverletzung verurteilt zu werden?",
        "• <b>Theoretisch ja:</b> Wird die App in den USA vertrieben, kann die unerlaubte Handlung dort eingeklagt werden.\n"
        "• <b>Faktischer Schutz:</b> Hat der Entwickler kein Vermögen in den USA, müsste der Patent-Troll das Urteil in Deutschland vollstrecken lassen. Dieses Verfahren ist extrem teuer und aufwendig, weshalb kleine Entwickler selten tatsächlich belangt werden."
    ),
    (
        "Welcher Zeitpunkt entscheidet bei einer Steuersatzänderung (z. B. Senkung/Erhöhung der MwSt) über den anzuwendenden Prozentsatz?",
        "Ausschließlich der **Zeitpunkt der Leistungserbringung** (nicht der Rechnungsstellung oder des Zahlungseingangs).\n"
        "Wurde eine Leistung im Juni (bei 19% MwSt) erbracht, aber erst im August (bei z. B. 16% MwSt) abgerechnet, müssen in der Rechnung 19% Umsatzsteuer ausgewiesen werden."
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = '015_IT_Recht_fuer_Softwareentwickler.apkg'

# Save to root directory
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' im Hauptverzeichnis.")

# Also save to Managementkompetenz/itEnterpreneurship/Folien/
folien_dir = os.path.join('Managementkompetenz', 'itEnterpreneurship', 'Folien')
if os.path.exists(folien_dir):
    dest_path = os.path.join(folien_dir, output_filename)
    genanki.Package(anki_deck).write_to_file(dest_path)
    print(f"Erfolgreich kopiert nach: '{dest_path}'.")
