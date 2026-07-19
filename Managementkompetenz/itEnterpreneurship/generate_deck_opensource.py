import genanki
import os

MODEL_ID = 1829384790
DECK_ID = 2938475650

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
    'ITES Clean Model 017',
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
    'IT-Entrepreneurship :: 017 Rechtliche Rahmenbedingungen Open Source'
)

flashcards = [
    # ===== 1. Ursprung & Geschichte =====
    (
        "Was ist der historische Hintergrund für das Entstehen von freier und proprietärer Software in den 1970er und 1980er Jahren?",
        "• <b>Bis in die 70er / Anfang 80er:</b> Umgang mit Software war durch einen regen und offenen Austausch von Quelltexten zwischen Entwicklern und Nutzern geprägt.\n"
        "• <b>Ab den 80ern:</b> Es entstand ein Markt für proprietäre Software unter stark beschränkenden Lizenzen; Quelltexte wurden unter Verschluss gehalten (der 'Proprietär' = Eigentümer)."
    ),
    (
        "Auf wen geht die Ursprungsidee der „Freien Software“ (Free Software) zurück, wann wurde die FSF gegründet und was war das Ziel?",
        "• <b>Ursprungsidee:</b> Geht auf <b>Richard Stallman</b> zurück (bis 1984 beim MIT).\n"
        "• <b>FSF:</b> 1983 Gründung der <i>Free Software Foundation</i>.\n"
        "• <b>Ziel:</b> Die Erlaubnis zur Veränderung und Erstellung abgeleiteter Werke (Derivate) sollte die Evolution des Quellcodes ermöglichen und damit für mehr Innovation sorgen."
    ),
    (
        "Was ist das GNU-Projekt und wofür steht das Akronym?",
        "• <b>Definition:</b> Ein 1985 gestartetes Projekt zur Schaffung eines freien, unixähnlichen Betriebssystems auf Basis freier Software.\n"
        "• <b>Akronym:</b> Steht rekursiv für <b>„GNU’s Not Unix“</b> (das erste Initial kürzt das gesamte Akronym ab)."
    ),
    (
        "Was versteht man unter proprietärer Software?",
        "Software, die das Recht und die Möglichkeiten der Wieder- und Weiterverwendung sowie Änderung und Anpassung durch Nutzer und Dritte stark einschränkt.\n"
        "<i>Hintergrund:</i> Eigentumsrecht liegt beim Eigentümer (der Proprietär)."
    ),
    (
        "Wann wurde der Begriff „Open Source“ geprägt, durch wen, und was war die Zielsetzung im Vergleich zur FSF?",
        "• <b>Begriffsprägung:</b> 1998 durch die <b>Open Source Initiative (OSI)</b>.\n"
        "• <b>Zielsetzung:</b> Förderung von Open-Source-Software zur Erleichterung der kommerziellen Nutzung.\n"
        "• <b>Abgrenzung zur FSF:</b> Reine Marketinggründe zur begrifflichen Abgrenzung von der FSF, welche von mehr politisch-sozialer Ideologie geprägt ist."
    ),
    (
        "Was ist die rechtliche Grundlage für Open-Source-Software?",
        "Das <b>Urheberrecht</b>.\n"
        "Ohne Urheberrecht gibt es kein Open Source, da der Urheber das ausschließliche Recht zur Nutzung, Vervielfältigung und Verbreitung besitzt und dieses Recht über die OS-Lizenz gezielt an andere einräumt (bzw. auf die Exklusivität verzichtet)."
    ),

    # ===== 2. Die Open Source Definition (OSD) =====
    (
        "Was ist die Open Source Definition (OSD) und wie viele Merkmale verlangt sie?",
        "Die OSD ist keine Lizenz, sondern ein Standard der OSI, an dem Lizenzen auf Konformität gemessen werden. Sie verlangt das Erfüllen aller <b>10 Merkmale</b> der Definition (Version 1.9)."
    ),
    (
        "Was besagt das OSD-Merkmal 1 („Freie Weitergabe“) bezüglich Lizenzgebühren und Verkauf?",
        "Die Lizenz darf niemanden darin einschränken, die Software als Teil eines Pakets zu verschenken oder zu verkaufen.\n"
        "Es dürfen **keinerlei Lizenz- oder Nutzungsgebühren** pro Installation verlangt werden (Vertriebskosten für z.B. das Pressen von CDs/Hosting dürfen erhoben werden, aber keine reinen Lizenzkosten)."
    ),
    (
        "Was besagen die OSD-Merkmale 2 bis 4 bezüglich Quellcode, abgeleiteten Arbeiten und Autoren-Quellcode?",
        "• <b>2. Verfügbarer Quellcode:</b> Die Software muss im Quellcode für alle Nutzer verfügbar sein.\n"
        "• <b>3. Abgeleitete Arbeiten:</b> Die Lizenz muss Modifikationen (Derivate) und deren Distribution unter derselben Lizenz erlauben.\n"
        "• <b>4. Integrität des Autoren-Quellcodes:</b> Modifizierter Quellcode darf verteilt werden. Die Lizenz darf verlangen, dass solche Änderungen zu einem neuen Namen/Versionsnummer führen oder als Patches geliefert werden."
    ),
    (
        "Darf eine Open-Source-Lizenz die Nutzung für bestimmte Personen oder Zwecke (z. B. militärisch, kommerziell) verbieten?",
        "<b>Nein, das ist ausgeschlossen.</b>\n"
        "• <b>5. Keine Diskriminierung von Personen/Gruppen:</b> Keine Verweigerung (z. B. für Bürger bestimmter Staaten).\n"
        "• <b>6. Keine Nutzungseinschränkung:</b> Der Verwendungszweck darf nicht eingeschränkt werden (z. B. kein Ausschluss kommerzieller oder militärischer Nutzung)."
    ),
    (
        "Was verlangen die OSD-Merkmale 7 bis 10 bezüglich Lizenzerteilung, Produkt- und Technologieneutralität?",
        "• <b>7. Lizenzerteilung:</b> Die Lizenz muss für alle Empfänger gelten, ohne dass sie eine Registrierung oder Zusatzlizenz erwerben müssen.\n"
        "• <b>8. Produktneutralität:</b> Die Lizenz darf sich nicht auf eine bestimmte Distribution (Produktpaket) beziehen.\n"
        "• <b>9. Keine Einschränkung anderer Software:</b> Die Lizenz darf nicht verlangen, dass andere mitgelieferte Software ebenfalls Open Source sein muss.\n"
        "• <b>10. Technologieneutral:</b> Keine Distributionseinschränkung auf ein bestimmtes Medium (z. B. nur per CD/Web)."
    ),

    # ===== 3. Copyleft & Lizenztypen =====
    (
        "Was ist die Copyleft-Klausel in Open-Source-Lizenzen?",
        "Eine Klausel in urheberrechtlichen Lizenzen, die den Lizenznehmer verpflichtet, **jegliche Bearbeitung des Werks** (Erweiterung, Veränderung) bei Weitergabe unter die **gleiche Lizenz** des ursprünglichen Werks zu stellen.\n"
        "<i>Zweck:</i> Verhindert, dass veränderte Fassungen des Werks mit Nutzungseinschränkungen (unfrei) weitergegeben werden."
    ),
    (
        "Was unterscheidet das kontinentaleuropäische Urheberrecht vom amerikanischen Copyright?",
        "• <b>Urheberrecht:</b> Schützt den Schöpfer; das Urheberrecht als Ganzes ist nicht übertragbar (Verzicht auf das Urheberrecht zu Lebzeiten ist unzulässig). Es können nur Nutzungsrechte eingeräumt werden.\n"
        "• <b>Copyright:</b> Ein ökonomisch geprägtes Verwertungsrecht, das vollständig (auch an Unternehmen) abgetreten und übertragen werden kann."
    ),
    (
        "In welche drei Hauptkategorien lassen sich Open-Source-Lizenzen einteilen? Nenne jeweils ein Beispiel.",
        "1. <b>Strenge Copyleft-Lizenzen:</b> Jede Modifikation und Weitergabe muss unter der gleichen Lizenz erfolgen (z. B. GPL, AGPL).\n"
        "2. <b>Beschränkte Copyleft-Lizenzen:</b> Erlauben die Kombination mit Code unter anderen (auch proprietären) Lizenzen in eigenen Dateien (z. B. LGPL).\n"
        "3. <b>Non-Copyleft-Lizenzen (Permissive Lizenzen):</b> Keine Pflicht zur Weitergabe unter derselben Lizenz; Derivate können proprietär werden (z. B. BSD, MIT)."
    ),
    (
        "Unter welchen Bedingungen darf Software unter einer strengen Copyleft-Lizenz (z. B. GPL) modifiziert werden, ohne den Quellcode zu veröffentlichen?",
        "Solange die Software **ausschließlich intern (privat oder firmenintern)** genutzt und nicht an Dritte weitergegeben wird.\n"
        "Die Veröffentlichungspflicht (Copyleft) greift erst im Moment des Vertriebs oder der Weitergabe der ausführbaren Software an Endnutzer."
    ),
    (
        "Wie schützt die GPL v3 das Open-Source-Projekt vor der Aushöhlung durch Softwarepatente?",
        "Wer Software unter der GPL v3 vertreibt, muss der **lizenzkostenfreien Nutzung eigener Patente** zustimmen, die diese Software betreffen.\n"
        "Dadurch wird ein kostenpflichtiger Huckepackvertrieb von Patenten mittels GPL-lizenziertem Code ausgeschlossen."
    ),
    (
        "Was ist das „ASP-Schlupfloch“ (Hosting-Lücke) bei der GPL und wie wird es durch die AGPL geschlossen?",
        "• <b>Lücke:</b> Die GPL greift nur bei der <i>Weitergabe</i> (Distribution) von Kopien. Wird eine modifizierte GPL-Software nur auf eigenen Servern als Webdienst (ASP/SaaS) betrieben, liegt keine Weitergabe vor und der Quelltext muss nicht geteilt werden.\n"
        "• <b>AGPL-Lösung:</b> Die <i>Affero General Public License</i> schließt dies, indem sie Nutzern, die die Software über ein Netzwerk nutzen, eine Downloadmöglichkeit des vollständigen Quellcodes garantiert."
    ),
    (
        "Nenne drei bekannte Softwareprodukte, die unter der AGPL stehen.",
        "• <b>MongoDB</b> (Dokumentenorientierte NoSQL-Datenbank)\n"
        "• <b>Nextcloud</b> (freie Cloudsoftware)\n"
        "• <b>Bacula</b> (Datensicherungsprogramm)\n"
        "• <b>Nuclos</b> (ERP-Baukasten)\n"
        "• <b>OTRS</b> (Ticketsystem)"
    ),
    (
        "Was erlaubt die LGPL (Lesser GPL) bei der Einbindung in proprietäre Software und wie wird dies technisch realisiert?",
        "• <b>Erlaubnis:</b> Die LGPL erlaubt das Einbinden von LGPL-Software in eigene proprietäre Software, ohne den eigenen Quellcode offenlegen zu müssen.\n"
        "• <b>Technische Realisierung:</b> Die LGPL-Teile werden meist als <b>dynamische Programmierbibliothek (z. B. DLL / .so)</b> eingebunden, damit Endnutzer die LGPL-Teile austauschen und anpassen können (notwendige Trennung)."
    ),
    (
        "Welche Besonderheit gilt bei der Weitergabe von Modifikationen an Software unter Non-Copyleft-Lizenzen (z. B. BSD, MIT)?",
        "Abgeleitete Werke (Derivate) müssen **nicht** unter derselben Lizenz stehen. Es ist erlaubt, nur Binärdateien weiterzugeben und die Software in **proprietäre Software** zu überführen (Übergang in Closed Source möglich)."
    ),
    (
        "Was ist die EUPL (European Union Public Licence) und mit welcher Lizenz ist sie kompatibel?",
        "• <b>Definition:</b> Erste einheitliche Open-Source-Lizenz für alle EU-Länder, kompatibel zu den jeweiligen nationalen Urheberrechten.\n"
        "• <b>Zweck:</b> Ursprünglich für Software von europäischen Behörden („Public Money? Public Code!“).\n"
        "• <b>Kompatibilität:</b> Kompatibel zu GPL v2."
    ),

    # ===== 4. Weitere Softwarearten & Rechtliches =====
    (
        "Was ist Public Domain Software und wie unterscheidet sich die rechtliche Lage in Deutschland von anderen Ländern?",
        "• <b>Konzept:</b> Der Urheber gibt sämtliche Rechte an die Allgemeinheit ab.\n"
        "• <b>Lage in Deutschland:</b> Ein vorzeitiger Verzicht auf das Urheberrecht ist gesetzlich **nicht vorgesehen** (Erlöschen erst 70 Jahre nach Tod). Willenserklärungen wie die „Do what the fuck you want to public licence“ (WTFPL) werden aber als sehr weitreichende Nutzungsrechte interpretiert."
    ),
    (
        "Grenze Freeware und Shareware voneinander ab.",
        "• <b>Freeware:</b> Umfassendes kostenfreies Nutzungs- und Weiterverbreitungsrecht, aber Quellcode bleibt geschlossen und Änderungen sind untersagt (Freeware ≠ Free Software).\n"
        "• <b>Shareware:</b> Freie Nutzung nur für eine bestimmte Zeit (z. B. 30 Tage) oder in bestimmtem Umfang zur Probe, danach ist der Erwerb einer Lizenz erforderlich."
    ),
    (
        "Warum ist der standardmäßige, vollständige Haftungsausschluss in OS-Lizenzen nach deutschem Recht problematisch und wie wird dies gelöst?",
        "• <b>Problem:</b> Open-Source-Lizenzen werden als AGB eingestuft. Ein vollständiger Haftungs- und Gewährleistungsausschluss ist im deutschen AGB-Recht unwirksam.\n"
        "• <b>Lösung:</b> Die kostenlose Überlassung wird als **Schenkung** eingestuft. Bei einer Schenkung haftet der Schenkende gesetzlich nur für Vorsatz und arglistig verschwiegene Fehler. In der Praxis entfällt damit das Haftungsrisiko."
    ),
    (
        "Welches Haftungsrisiko besteht, wenn man in Deutschland eigene Freeware zur Verfügung stellt?",
        "Da die Überlassung kostenfrei erfolgt, wird sie rechtlich meist als **Leihvertrag** (§ 598 BGB) interpretiert. Bei der Leihe haftet der Verleiher nur für Vorsatz und grobe Fahrlässigkeit. Das Haftungsrisiko ist in der Praxis also minimal."
    ),
    (
        "Welche Pflicht gilt für Rechteinhaber bezüglich einer eingetragenen Marke für Open-Source-Software?",
        "Die Marke muss **rechtserhaltend genutzt** werden (§ 26 MarkenG).\n"
        "Es müssen ernsthafte geschäftliche Tätigkeiten entfaltet werden, die der Vermarktung und Erhöhung des Verbreitungsgrads dienen, da die Marke ansonsten wegen Nichtbenutzung verfallen kann."
    ),
    (
        "Welche Grundregel gilt bei der Kombination verschiedener Open-Source-Lizenzen zu einem gemeinsamen Produkt?",
        "Code kann grundsätzlich nur **weniger-freigiebig** lizenziert werden.\n"
        "Beispiel: BSD-Code (permissive) kann problemlos in ein GPL-Projekt (strenger Copyleft) überführt werden, da die GPL strengere Bedingungen vorschreibt, die den BSD-Bedingungen nicht widersprechen."
    ),
    (
        "Wann stellt die Anbindung einer GPL-Bibliothek an ein kommerzielles Closed-Source-Produkt ein Problem dar und wann ist es legitim (Workaround)?",
        "• <b>Problem (GPL-Verletzung):</b> Wenn GPL-Code statisch/dynamisch in das proprietäre Produkt gelinkt oder mitkompiliert wird.\n"
        "• <b>Legitim:</b> Wenn das proprietäre Produkt ein GPL-Programm als eigenständigen Prozess zur Laufzeit über lose, standardisierte Schnittstellen aufruft (z. B. Datenaustausch über JSON/XML, Pipes oder Kommandozeilenparameter)."
    )
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = '017_Rechtliche_Rahmenbedingungen_OpenSource.apkg'

# Save to root directory
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' im Hauptverzeichnis.")

# Also save to Managementkompetenz/itEnterpreneurship/Folien/
folien_dir = os.path.join('Managementkompetenz', 'itEnterpreneurship', 'Folien')
if os.path.exists(folien_dir):
    dest_path = os.path.join(folien_dir, output_filename)
    genanki.Package(anki_deck).write_to_file(dest_path)
    print(f"Erfolgreich kopiert nach: '{dest_path}'.")
