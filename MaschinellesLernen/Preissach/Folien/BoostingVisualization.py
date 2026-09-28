import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from sklearn.ensemble import AdaBoostClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn.datasets import make_moons

# 1. Künstlichen, leicht verschachtelten Datensatz erzeugen (z. B. Moon-Data)
X, y = make_moons(n_samples=60, noise=0.2, random_state=42)
y = np.where(y == 0, -1, 1)  # Für AdaBoost sind Klassen oft -1 und 1

num_iterations = 10
N = len(X)

# Gewichte initialisieren (Schritt 1: Alle haben das gleiche Gewicht 1/N)
weights = np.ones(N) / N

# Listen, um den Verlauf für die Animation zu speichern
all_weights = [weights.copy()]
all_stumps = []
all_errors = []

# Manueller AdaBoost-Durchlauf zur Aufzeichnung der Schritte
for s in range(num_iterations):
    # Trainiere einen extrem einfachen Baum (Tiefe 1 = Decision Stump) mit aktuellen Gewichten
    stump = DecisionTreeClassifier(max_depth=1, random_state=42)
    stump.fit(X, y, sample_weight=weights)
    
    # Vorhersage treffen
    predictions = stump.predict(X)
    
    # Fehler berechnen (Summe der Gewichte der falsch klassifizierten Punkte)
    misclassified = (y != predictions)
    err_s = np.sum(weights[misclassified]) / np.sum(weights)
    
    # Konfidenz-Koeffizient w_s berechnen
    # Vermeide Division durch Null, falls err_s = 0
    err_s = max(err_s, 1e-10)
    w_s = 0.5 * np.log((1.0 - err_s) / err_s)
    
    # Gewichte aktualisieren (Falsche werden schwerer, Richtige leichter)
    weights *= np.exp(-w_s * y * predictions)
    weights /= np.sum(weights)  # Normalisieren
    
    # Speichern für die Animation
    all_weights.append(weights.copy())
    all_stumps.append(stump)
    all_errors.append(err_s)

# 2. Plot und Animation aufsetzen
fig, ax = plt.subplots(figsize=(10, 7))

# Erzeuge ein Gitternetz (Grid) für die zeichnerische Darstellung der Entscheidungsgrenzen
xx, yy = np.meshgrid(np.linspace(-2, 3, 200), np.linspace(-1.5, 2, 200))

def update(frame):
    ax.clear()
    
    current_stump = all_stumps[frame]
    current_weights = all_weights[frame]
    
    # Hintergrund-Entscheidungsgrenze des aktuellen Stumps zeichnen
    Z = current_stump.predict(np.c_[xx.ravel(), yy.ravel()])
    Z = Z.reshape(xx.shape)
    ax.contourf(xx, yy, Z, alpha=0.2, cmap='bwr')
    
    # Skaliere die Punktgröße im Plot basierend auf dem aktuellen Gewicht
    # Multiplikation mit 1500, damit man den Unterschied gut mit dem Auge sieht
    point_sizes = current_weights * 1500
    
    # Datenpunkte zeichnen (Klasse -1 in Blau, Klasse 1 in Rot)
    ax.scatter(X[y == -1, 0], X[y == -1, 1], c='blue', s=point_sizes[y == -1], 
               edgecolors='k', label='Klasse -1', alpha=0.7)
    ax.scatter(X[y == 1, 0], X[y == 1, 1], c='red', s=point_sizes[y == 1], 
               edgecolors='k', label='Klasse 1', alpha=0.7)
    
    ax.set_title(f"AdaBoost Iteration {frame + 1}/{num_iterations}\n"
                 f"Fehlerrate des aktuellen Stumps: {all_errors[frame]:.3f}", fontsize=14)
    ax.set_xlabel("Merkmal 1")
    ax.set_ylabel("Merkmal 2")
    ax.legend(loc='upper left')
    ax.grid(True, linestyle='--', alpha=0.5)

# Animation starten
ani = FuncAnimation(fig, update, frames=num_iterations, interval=1500, repeat=True)

plt.show()