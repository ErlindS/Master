import numpy as np
import matplotlib.pyplot as plt

# Konfiguration für eine saubere Anzeige der Zahlen in den Kästchen
def plot_matrix(ax, matrix, title, cmap="YlGnBu"):
    im = ax.imshow(matrix, cmap=cmap, vmin=matrix.min()-1, vmax=matrix.max()+1)
    ax.set_title(title, fontsize=12, fontweight='bold', pad=10)
    
    # Zahlen in die Kästchen schreiben
    for i in range(matrix.shape[0]):
        for j in range(matrix.shape[1]):
            ax.text(j, i, f"{int(matrix[i, j])}", ha="center", va="center", 
                    color="black" if matrix[i, j] < matrix.max()/2 else "white",
                    fontweight='bold')
            
    # Achsen-Striche verschönern
    ax.set_xticks(np.arange(matrix.shape[1]))
    ax.set_yticks(np.arange(matrix.shape[0]))
    ax.set_xticklabels(np.arange(1, matrix.shape[1] + 1))
    ax.set_yticklabels(np.arange(1, matrix.shape[0] + 1))

# ==========================================
# 1. DAS ORIGINALBILD (6x6 Pixel)
# Wir bauen ein Muster: Links oben hell (10), rechts unten hell (10)
# ==========================================
original = np.array([
    [10, 10, 10,  0,  0,  0],
    [10, 10, 10,  0,  0,  0],
    [10, 10, 10,  0,  0,  0],
    [ 0,  0,  0, 10, 10, 10],
    [ 0,  0,  0, 10, 10, 10],
    [ 0,  0,  0, 10, 10, 10]
])

# ==========================================
# 2. PADDING (Rahmen aus Nullen hinzufügen)
# Wir fügen einen Rahmen von 1 Pixel Dicke hinzu (P=1)
# ==========================================
padded = np.pad(original, pad_width=1, mode='constant', constant_values=0)

# ==========================================
# 3. DER FILTER / KERNEL (3x3 Schablone)
# Dieser Filter reagiert stark auf vertikale Kanten
# ==========================================
kernel = np.array([
    [ 1,  0, -1],
    [ 1,  0, -1],
    [ 1,  0, -1]
])

# ==========================================
# 4. DIE FALTUNG (Convolution mit Stride = 1)
# Wir schieben den Filter über das gepaddete Bild. 
# Weil wir Padding genutzt haben, bleibt das Ergebnis genau 6x6 groß!
# ==========================================
conv_out = np.zeros((6, 6))
for i in range(6):
    for j in range(6):
        # Wir schneiden das 3x3-Fenster aus dem gepaddeten Bild aus
        region = padded[i:i+3, j:j+3]
        # Multiplizieren mit der Schablone und alles zusammenrechnen
        conv_out[i, j] = np.sum(region * kernel)

# ==========================================
# 5. MAX POOLING (2x2 Blöcke, Stride = 2)
# Der Chef-Redakteur sucht aus jedem 2x2 Block nur die lauteste Zahl
# ==========================================
pool_out = np.zeros((3, 3))
for i in range(0, 6, 2):
    for j in range(0, 6, 2):
        region = conv_out[i:i+2, j:j+2]
        pool_out[i//2, j//2] = np.max(region)

# ==========================================
# VISUALISIERUNG
# ==========================================
fig, axes = plt.subplots(2, 2, figsize=(12, 10))

# Plot 1: Das Original
plot_matrix(axes[0, 0], original, "1. Originales Bild (6x6)\n(Muster mit Intensität 10)")

# Plot 2: Das gepaddete Bild
plot_matrix(axes[0, 1], padded, "2. Padding (8x8)\n(Künstlicher Rahmen aus 0en hinzugefügt)")

# Plot 3: Nach der Faltung
plot_matrix(axes[1, 0], conv_out, "3. Nach Faltung (6x6)\n(Filter hat Kanten berechnet)", cmap="coolwarm")

# Plot 4: Nach dem Max-Pooling
plot_matrix(axes[1, 1], pool_out, "4. Nach Max-Pooling (3x3)\n(Größe halbiert, nur Maxima überleben)", cmap="coolwarm")

plt.tight_layout()
plt.show()