import genanki

MODEL_ID = 1710928348
DECK_ID = 2098431129

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
"""

model = genanki.Model(
    MODEL_ID,
    'Uebungen Clean Model',
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
    'Maschinelles Lernen :: Übungen & Klausurwissen'
)

flashcards = [
    # ===== 1. Distanz- und Ähnlichkeitsmaße =====
    (
        "Nenne die Formeln für die euklidische, Cityblock- und Chebyshev-Distanz. In welcher Reihenfolge stehen sie immer?",
        "<b>Euklidische (L₂):</b> \\(d_2 = \\sqrt{\\sum_k (x_k - y_k)^2}\\)\n\n<b>Cityblock (L₁):</b> \\(d_1 = \\sum_k |x_k - y_k|\\)\n\n<b>Chebyshev (L∞):</b> \\(d_\\infty = \\max_k |x_k - y_k|\\)\n\n<b>Reihenfolge (gilt immer):</b>\n\\[ d_\\infty \\leq d_2 \\leq d_1 \\]\nChebyshev ≤ Euklidisch ≤ Cityblock"
    ),
    (
        "Welche drei Eigenschaften muss eine Distanzfunktion erfüllen, um eine Metrik zu sein? Ist DTW eine Metrik?",
        "1. <b>Positive Definitheit:</b> \\(d(\\vec{x},\\vec{y}) \\geq 0\\) und \\(d(\\vec{x},\\vec{y}) = 0 \\Leftrightarrow \\vec{x} = \\vec{y}\\)\n2. <b>Symmetrie:</b> \\(d(\\vec{x},\\vec{y}) = d(\\vec{y},\\vec{x})\\)\n3. <b>Dreiecksungleichung:</b> \\(d(\\vec{x},\\vec{z}) \\leq d(\\vec{x},\\vec{y}) + d(\\vec{y},\\vec{z})\\)\n\n<b>DTW ist keine Metrik!</b> Verletzt die Dreiecksungleichung und \\(d(\\vec{s},\\vec{t}) = 0\\) kann für \\(\\vec{s} \\neq \\vec{t}\\) gelten (gestreckte Versionen). DTW ist ein Distanzmaß, keine Metrik."
    ),
    (
        "Kovarianzmatrix berechnen: Was ist das Rezept und der typischste Fehler?",
        "<b>Rezept:</b>\n1. <b>Mittelwerte</b> berechnen\n2. <b>Zentrieren</b> (Mittelwert abziehen)\n3. Kovarianzmatrix: \\(C = \\frac{1}{N-1} G G^T\\)\n\n<b>Typischster Fehler:</b> Nenner <b>N statt N−1</b>!\n\nDie Kovarianz ist <b>skalenabhängig</b> → nur Richtungsaussage.\nFür Stärke: <b>Pearson-Korrelation</b> \\(r = \\frac{\\text{Cov}(X,Y)}{\\sigma_X \\sigma_Y} \\in [-1,1]\\)"
    ),
    (
        "Was ist die Mahalanobis-Distanz und wann verwendet man sie?",
        "<b>Formel:</b> \\(d_C(\\vec{x}, \\vec{y}) = \\sqrt{(\\vec{x}-\\vec{y})^T C^{-1} (\\vec{x}-\\vec{y})}\\)\n\n<b>Berechnung bei Diagonalmatrix:</b> Kehrwerte auf der Diagonale!\n\n<b>Interpretation:</b> Punkte gleicher Mahalanobis-Distanz liegen auf <b>Ellipsen</b>, die der Datenform folgen. Richtungen mit hoher Varianz werden heruntergewichtet.\n\n<b>Verwenden bei:</b> Stark <b>korrelierte Merkmale</b> mit verschiedenen Einheiten."
    ),
    (
        "DTW-Berechnung: Was ist das Rezept und was sind die drei Randbedingungen?",
        "<b>Rezept:</b>\n1. Lokale Kosten berechnen: \\(d'_{ij} = |s_i - t_j|\\)\n2. Ränder mit ∞ initialisieren\n3. Kumulierte Kosten: \\(d_{ij} = d'_{ij} + \\min(d_{i-1,j}, d_{i,j-1}, d_{i-1,j-1})\\)\n4. DTW-Distanz = Wert rechts oben\n\n<b>Drei Randbedingungen:</b>\n1. <b>Randbedingung:</b> Anfang auf Anfang, Ende auf Ende\n2. <b>Monotonie:</b> Indizes monoton steigend, jeder Index mindestens einmal\n3. <b>Schrittweite:</b> Nur (0,1), (1,0), (1,1) erlaubt"
    ),
    (
        "SMC vs. Jaccard: Wann welches Maß? Und wann Kosinus-Ähnlichkeit?",
        "<b>SMC</b> = \\(\\frac{f_{11} + f_{00}}{\\text{alle}}\\) — zählt gemeinsame Nullen mit\n\n<b>Jaccard</b> = \\(\\frac{f_{11}}{f_{11} + f_{10} + f_{01}}\\) — ignoriert gemeinsame Nullen\n\n<b>Jaccard verwenden bei:</b> Dünn besetzten Daten (z.B. Warenkörbe) — gemeinsame Nullen sind nicht aussagekräftig!\n\n<b>Kosinus-Ähnlichkeit:</b> \\(\\cos(\\vec{x},\\vec{y}) = \\frac{\\vec{x}^T \\vec{y}}{\\|\\vec{x}\\| \\|\\vec{y}\\|}\\)\n<b>Verwenden bei:</b> BOW-Dokumentvektoren, Embeddings — <b>längeninvariant</b>, Inhalt zählt, nicht Dokumentlänge."
    ),
    (
        "k-NN Klassifikation: Wie geht man vor und welcher Trick spart Rechenzeit?",
        "<b>Vorgehen:</b>\n1. Distanz zu allen Trainingspunkten berechnen\n2. k nächste Nachbarn finden\n3. <b>Mehrheitsentscheid</b> der k Nachbarn\n\n<b>Trick:</b> Man kann <b>quadrierte</b> euklidische Distanzen vergleichen (Wurzel weglassen!), da die Wurzel <b>monoton</b> ist und die Rangfolge nicht ändert."
    ),

    # ===== 2. Clustering =====
    (
        "SSE berechnen: Was ist das Rezept und der typischste Fehler?",
        "<b>Rezept:</b>\n1. <b>Zentren</b> berechnen (Mittelwert pro Koordinate pro Cluster)\n2. <b>Quadrierte</b> Abstände jedes Punktes zu seinem Zentrum summieren\n\n\\[ SSE = \\sum_k \\sum_{\\vec{g} \\in C_k} \\|\\vec{g} - \\vec{z}_k\\|^2 \\]\n\n<b>Typischer Fehler:</b> <b>Keine Wurzel</b> beim SSE! Es werden quadrierte Abstände summiert.\n\nDer Schwerpunkt (Mittelwert) ist die SSE-optimale Position des Zentrums (Beweis: Ableitung = 0)."
    ),
    (
        "Charakterisiere k-Means anhand der vier Begriffspaare.",
        "k-Means ist:\n• <b>Partitionierend</b> (feste Aufteilung, kein Baum)\n• <b>Exklusiv</b> (jeder Punkt genau ein Cluster)\n• <b>Vollständig</b> (jeder Punkt wird zugeordnet)\n• <b>K fest vorgegeben</b>\n\n<b>Zwei Gründe für unterschiedliche Ergebnisse:</b>\n1. <b>Zufällige Initialisierung</b> → Konvergenz in unterschiedliche lokale Minima\n2. Zufällige Reihenfolge / leere Cluster\n\n<b>Abhilfe:</b> Mehrfacher Lauf, bestes SSE wählen."
    ),
    (
        "Silhouette berechnen und interpretieren.",
        "<b>Formel:</b> \\(s(\\vec{x}) = \\frac{b - a}{\\max(a, b)}\\)\n\n• \\(a\\) = mittlerer Abstand zum <b>eigenen</b> Cluster\n• \\(b\\) = mittlerer Abstand zum <b>nächsten fremden</b> Cluster\n\n<b>Interpretation:</b>\n• \\(s \\approx +1\\): Punkt passt gut ins Cluster\n• \\(s \\approx 0\\): Grenzfall zwischen zwei Clustern\n• \\(s \\approx -1\\): Punkt vermutlich <b>falsch zugeordnet</b>\n\nSilhouettenkoeffizient = Mittel über alle Punkte → zur <b>Wahl von K</b> geeignet."
    ),
    (
        "DBSCAN: Was sind Kern-, Rand- und Rauschpunkte? Welche Relationen sind symmetrisch?",
        "<b>Kernpunkt:</b> Mehr als MinPts Punkte in der ε-Umgebung (inkl. selbst)\n<b>Randpunkt:</b> In ε-Umgebung eines Kernpunkts, aber selbst kein Kern\n<b>Rauschpunkt:</b> Weder Kern noch erreichbar\n\n<b>Relationen:</b>\n• <b>Direkt erreichbar:</b> \\(\\vec{x} \\in U_\\varepsilon(\\vec{y})\\) und \\(\\vec{y}\\) ist Kernobjekt — <b>nur für Kernobjekte symmetrisch</b>\n• <b>Erreichbar:</b> Kette direkter Erreichbarkeiten — <b>nur für Kernobjekte symmetrisch</b>\n• <b>Verbunden:</b> Beide von einem gemeinsamen Punkt erreichbar — <b>immer symmetrisch</b>\n\nDBSCAN findet nichtkonvexe Cluster (vs. k-Means: nur Voronoi-Zellen)."
    ),
    (
        "Single, Complete, Average, Centroid Linkage: Ordne die Definitionen zu.",
        "<b>Single Linkage:</b> <b>Minimaler</b> paarweiser Abstand zwischen den Clustern\n\n<b>Complete Linkage:</b> <b>Maximaler</b> paarweiser Abstand\n\n<b>Average Linkage:</b> <b>Durchschnitt</b> aller Paare\n\n<b>Centroid Linkage:</b> Abstand der <b>Schwerpunkte</b>\n\n<b>Medoid vs. Schwerpunkt:</b> Medoid ist ein <b>echter Datenpunkt</b> → robuster gegen Ausreißer. Algorithmus: <b>PAM</b> mit BUILD- und SWAP-Phase."
    ),

    # ===== 3. PCA =====
    (
        "PCA-Rechenaufgabe: Was ist das Rezept (7 Schritte)?",
        "1. <b>Mittelwert</b> berechnen\n2. <b>Zentrieren</b> (Mittelwert abziehen)\n3. <b>Kovarianzmatrix:</b> \\(C = \\frac{1}{N-1} G G^T\\)\n4. <b>Eigenwerte:</b> \\(\\det(C - \\lambda I) = 0\\) lösen, <b>absteigend sortieren!</b>\n5. <b>Eigenvektoren:</b> \\((C - \\lambda_i I) \\vec{e}_i = \\vec{0}\\) lösen und <b>normieren</b>\n6. <b>Transformieren:</b> \\(\\vec{g}' = U^T (\\vec{g} - \\vec{g}_{mean})\\)\n7. <b>Ggf. reduzieren + rekonstruieren</b>\n\n<b>Typische Fehler:</b> Zentrierung vergessen, N statt N−1, Eigenvektoren nicht normiert, bei Rücktransformation \\(\\vec{g}_{mean}\\) vergessen!"
    ),

    # ===== 4. Lineare Regression =====
    (
        "Loss-Funktion für lineare Regression aufstellen: Was ist das Rezept und was sind typische Fehler?",
        "<b>Rezept:</b> Pro Datenpunkt eine Klammer \\((y_i - (w_0 + w_1 x_{i1} + w_2 x_{i2}))^2\\), alles summieren, Faktor \\(\\frac{1}{N}\\) davor.\n\n<b>Kategorische Features:</b> Als 0/1 kodieren und die <b>Kodierung hinschreiben</b>!\n\n<b>Typische Fehler:</b>\n• \\(\\frac{1}{N}\\) vergessen\n• Kodierung nicht angegeben\n• Bei »w₀ weglassen« trotzdem w₀ hingeschrieben\n• Feature- und Labelspalte verwechselt\n\n<b>Optimalitätsbedingung:</b> Gradient = 0, d.h. \\(\\nabla L(w_0, w_1, w_2) = \\vec{0}\\)"
    ),
    (
        "Berechne MSE, RMSE, MAE und R²: Was bedeuten die Werte?",
        "<b>MSE</b> = \\(\\frac{1}{N} \\sum (y_i - \\hat{y}_i)^2\\)\n<b>RMSE</b> = \\(\\sqrt{MSE}\\) (in Originaleinheit)\n<b>MAE</b> = \\(\\frac{1}{N} \\sum |y_i - \\hat{y}_i|\\)\n\n<b>R²</b> = \\(1 - \\frac{RSS}{TSS}\\) mit RSS = \\(\\sum(y_i - \\hat{y}_i)^2\\), TSS = \\(\\sum(y_i - \\bar{y})^2\\)\n\n• R² = 1 → perfekt\n• R² = 0 → nicht besser als Mittelwert\n• R² &lt; 0 → schlechter als Mittelwert\n\n<b>Diagnose:</b> Train-R² hoch / Test-R² niedrig → <b>Overfitting</b> (Variance)\nTrain ≈ Test, beide mäßig → <b>Underfitting</b> (Bias)"
    ),

    # ===== 5. Logistische Regression =====
    (
        "Logistische Regression: Wie berechnet man Wahrscheinlichkeit, 50%-Grenze und Log-Loss? Was sind typische Fehler?",
        "<b>Wahrscheinlichkeit:</b> \\(\\hat{y} = \\sigma(w_0 + w^T x) = \\frac{1}{1+e^{-(w_0 + w^T x)}}\\)\n\n<b>50%-Grenze:</b> z = 0 setzen und nach Feature auflösen.\n\n<b>Vorzeichen-Trick:</b> z &gt; 0 → ŷ &gt; 0.5; z &lt; 0 → ŷ &lt; 0.5\n\n<b>Log-Loss (einzeln):</b>\n• y = 1: \\(-\\log(\\hat{y})\\)\n• y = 0: \\(-\\log(1 - \\hat{y})\\)\n\n<b>Typische Fehler:</b> Minuszeichen im Exponent verschlampt! \\(e^{-z}\\), nicht \\(e^z\\)!"
    ),
    (
        "Konfusionsmatrix: Wie berechnet man die Metriken und welche ist wann wichtig?",
        "<b>Erst TP/FP/FN/TN zählen!</b>\n\n• <b>Accuracy</b> = \\(\\frac{TP+TN}{N}\\)\n• <b>Precision</b> = \\(\\frac{TP}{TP+FP}\\) (Wie viel vom Vorhergesagten ist richtig?)\n• <b>Recall</b> = \\(\\frac{TP}{TP+FN}\\) (Wie viel vom Echten wurde gefunden?)\n\n<b>Merkhilfe:</b> Re<b>call</b> = wie viele echte Positive ab<b>gerufen</b> wurden\n\n<b>Precision erhöhen:</b> Schwellwert erhöhen (auf Kosten von Recall)\n\n<b>Accuracy ungeeignet</b> bei unbalancierten Klassen (Trivialklassifikator erreicht 99.9%)!"
    ),

    # ===== 6. Regularisierung =====
    (
        "Regularisierung: Was ist die Standardantwort für die Klausur? (kam in ALLEN 5 Klausuren!)",
        "<b>Standardantwort:</b>\n<b>Ziel</b> ist es, Overfitting (zu starkes Anpassen an die Trainingsdaten) zu vermeiden.\n<b>Erreicht</b> wird das, indem beim Training die Modellflexibilität <b>eingeschränkt</b> wird (z.B. Strafterm auf große Koeffizienten).\n\n<b>Ridge (ℓ₂):</b> Bestraft \\(\\sum w_j^2\\) → alle Koeffizienten verkleinert, selten = 0\n<b>Lasso (ℓ₁):</b> Bestraft \\(\\sum |w_j|\\) → viele Koeffizienten <b>exakt Null</b> → automatische Featureauswahl\n\n<b>Nachteile:</b>\n1. Trainingsgüte wird schlechter\n2. Parameter nicht mehr direkt interpretierbar"
    ),
    (
        "Gittersuche & Modelle zählen: Was ist das Rezept?",
        "<b>Rezept:</b>\n1. <b>Testset abziehen</b> → Trainingsmenge\n2. <b>Kandidaten</b> = (λ-Werte) × (Verfahren)\n3. <b>× k</b> bei k-facher CV\n4. Trainingsgröße pro Lauf = \\(\\frac{k-1}{k}\\) der Trainingsmenge\n5. <b>+ 1 finales Modell</b> auf ganzer Trainingsmenge nicht vergessen!\n\n<b>Beispiel:</b> 10.000 Punkte, 40% Test → 6.000 Training.\n10 λ-Werte × 2 Verfahren × 3-fold CV = <b>60 Modelle</b> mit je <b>4.000 Punkten</b>.\nDanach 1 finales Modell auf allen 6.000 Punkten."
    ),
    (
        "Nenne zwei Regularisierungsmethoden für neuronale Netze (neben ℓ₁/ℓ₂).",
        "1. <b>Dropout:</b> Zufälliges Deaktivieren von Neuronen während des Trainings\n2. <b>Early Stopping:</b> Training stoppen, wenn der Validierungsfehler wieder steigt (beginnendes Overfitting)\n\n<b>Early Stopping Vorgehen:</b>\n• Trainings- und Validierungsfehler beobachten\n• Anfangs sinken beide\n• Ab einem Punkt steigt der Validierungsfehler → <b>Stoppen</b>\n• Koeffizienten der Iteration mit geringstem Validierungsfehler laden"
    ),

    # ===== 7. Neuronale Netze =====
    (
        "Forward-Pass eines neuronalen Netzes: Was ist das Rezept und der typischste Fehler?",
        "<b>Rezept pro verdecktes Neuron:</b>\n1. <b>Gewichtete Summe</b> der Eingänge + Bias\n2. <b>ReLU</b> anwenden: \\(\\max(0, \\cdot)\\)\n3. Ergebnisse mit Output-Gewichten summieren (+ Output-Bias)\n\n<b>Matrizenform:</b> Zeile i von \\(W^{[1]}\\) = Gewichte, die zum i-ten Neuron führen.\n\n<b>Typischster Fehler:</b>\n<b>ReLU auf negative Summe nicht angewandt!</b>\n\\(\\max(0, -3) = 0\\), NICHT \\(-3\\)!"
    ),
    (
        "SGD-Dimensionen: Wie berechnet man Summanden, Gradientendimension und Updates?",
        "<b>Summanden der Verlustfunktion:</b> = N (ein pro Trainingsbeispiel, Featurezahl irrelevant!)\n\n<b>Gradientendimension:</b> = <b>Anzahl Parameter</b> (immer! auch bei Minibatch!)\n\n<b>Updates pro Epoche:</b> = \\(\\frac{N}{\\text{Batchgröße}}\\)\n\n<b>Updates gesamt:</b> = Updates/Epoche × Epochen\n\n<b>Batch-GD:</b> 1 Update pro Epoche\n\n<b>Warum SGD statt Batch-GD?</b>\n1. <b>Schneller</b> bei großen Datensätzen\n2. <b>Regularisierend</b>: Rauschen verhindert Overfitting"
    ),
    (
        "Parameter eines neuronalen Netzes zählen: Was ist die Formel?",
        "<b>Pro Schicht:</b> (Neuronen × Eingänge) + Neuronen (Bias)\n\n<b>Beispiel 784→128→64→10:</b>\n• 128 × 784 + 128 = 100.480\n• 64 × 128 + 64 = 8.256\n• 10 × 64 + 10 = 650\n• <b>Gesamt: 109.386</b>\n\n<b>Warum nichtlineare Aktivierung?</b> Ohne sie kollabiert das Netz zu einem linearen Modell, egal wie viele Schichten.\n\n<b>Universal Approximation Theorem:</b> 1 verdeckte Schicht reicht, um jede stetige Funktion zu approximieren. Garantiert aber NICHT, dass man die Gewichte effizient findet."
    ),

    # ===== 8. Backpropagation =====
    (
        "Backpropagation: Was ist das Rezept (immer gleich)?",
        "<b>Rezept:</b>\n1. <b>Graph:</b> Jede elementare Operation ein Knoten \\(x_i\\) mit Funktionsdefinition\n2. <b>Vorwärtsphase:</b> Werte aller Knoten an der gegebenen Stelle berechnen\n3. <b>Rückwärtsphase:</b> \\(\\bar{x}_N = 1\\), dann rückwärts:\n\\[ \\bar{x}_i = \\sum_{j \\in \\text{Kinder}} \\bar{x}_j \\cdot \\frac{\\partial x_j}{\\partial x_i} \\]\nWerte aus der Vorwärtsphase einsetzen!\n\n<b>Typische Fehler:</b>\n• Bei <b>Verzweigungen</b> (ein Knoten hat mehrere Kinder) die <b>Summe vergessen</b>\n• Ableitungen symbolisch statt numerisch eingesetzt\n• \\(\\frac{d}{dx}\\cos x = -\\sin x\\) (Vorzeichen!)"
    ),

    # ===== 9. CNN & Autoencoder =====
    (
        "CNN: Was sind die zwei wichtigsten Formeln?",
        "<b>Output-Dimension:</b>\n\\[ W_{out} = \\lfloor \\frac{W_{in} - W_K + 2P}{S} \\rfloor + 1 \\]\n\n<b>Parameter pro Conv-Layer:</b>\n\\[ \\#Param = K \\cdot (W_K \\cdot H_K \\cdot D_{in} + 1) \\]\n• K = Anzahl Filter\n• \\(D_{in}\\) = Tiefe der Eingabe (RGB → 3!)\n• +1 = Bias pro Filter\n\n<b>Typische Fehler:</b>\n• Bei RGB die Tiefe \\(D_{in} = 3\\) vergessen\n• Bias (+1 pro Filter) vergessen\n• <b>Pooling hat KEINE lernbaren Parameter!</b>\n• Stride ändert <b>nicht</b> die Parameterzahl, nur die Output-Dimension"
    ),
    (
        "Was ist ein Autoencoder und was ist sein Zusammenhang mit PCA?",
        "<b>Autoencoder:</b> NN das trainiert wird, seine <b>Eingabe zu rekonstruieren</b>. Mittlere Schicht (Bottleneck) hat weniger Neuronen → lernt reduzierte Darstellung.\n\n<b>Verlustfunktion:</b> Mittlerer quadratischer Rekonstruktionsfehler\n\\[ L = \\frac{1}{N} \\sum \\|x^{(i)} - x'^{(i)}\\|^2 \\]\n\n<b>Einsatz:</b> Dimensionsreduktion, Feature Learning, Anomalieerkennung\n\n<b>Zusammenhang mit PCA:</b>\nEin <b>linearer</b> Autoencoder (ohne Aktivierungsfunktionen, geteilte Gewichte) lernt <b>dieselbe Projektion wie die PCA</b> (bis auf Drehung/Spiegelung)."
    ),
    (
        "Max-Pooling: Was macht es und was ist das typische CNN-Architekturmuster?",
        "<b>Max-Pooling:</b> Prüft ob ein Feature <b>irgendwo in einer Region</b> vorhanden ist, verwirft die exakte Position → <b>Translationstoleranz</b>.\nKleinere Featuremaps → weniger Parameter.\n\n<b>CNN-Architekturmuster:</b>\nINPUT → [[CONV → RELU]*N → POOL?]*M → [FC → RELU]*K → FC\n\n<b>Arbeitsteilung:</b>\n• <b>Conv/Pool-Blöcke:</b> Feature Learning (Kanten → Formen → komplexe Muster)\n• <b>FC-Schichten:</b> Klassenentscheidung (wie lineares Modell auf gelernten Features)\n• <b>Letzte FC + Softmax:</b> Abbildung auf K Klassen"
    ),

    # ===== Zeitmanagement =====
    (
        "Klausur Zeitmanagement: Wie viel Zeit pro Punkt? Wie ist die Klausur aufgebaut?",
        "<b>Janka-Teil:</b> 60 Minuten, 46 Punkte → ca. <b>1,3 Minuten pro Punkt</b>\n\n<b>Typische Aufgabenverteilung (alle Klausuren):</b>\n• A1: Lineare/Logistische Regression (Vorhersage + Loss)\n• A2: Regularisierung + Gittersuche + Modelle zählen\n• A3: Forward-Pass neuronales Netz\n• A4: Backpropagation\n• A5: CNN (Dimensionen + Parameter) / Autoencoder\n\n<b>Tipp:</b> Unvereinfachte Brüche reichen oft!"
    ),
]

for q, a in flashcards:
    q_html = q.replace('\n', '<br>')
    a_html = a.replace('\n', '<br>')
    note = genanki.Note(model=model, fields=[q_html, a_html])
    anki_deck.add_note(note)

output_filename = 'ML_Uebungen_Klausurwissen.apkg'
genanki.Package(anki_deck).write_to_file(output_filename)
print(f"Erfolgreich generiert: '{output_filename}' mit {len(flashcards)} Karten.")