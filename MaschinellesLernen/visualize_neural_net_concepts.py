import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import matplotlib.patches as patches
import math

# Setup styling for a modern, dark look
plt.style.use('dark_background')
fig = plt.figure(figsize=(16, 9.8), facecolor='#121212')
fig.canvas.manager.set_window_title('Interaktive ML-Visualisierung: Aufgabe 3 (a & c)')

# Subplots
ax_left = fig.add_axes([0.05, 0.28, 0.44, 0.62], facecolor='#181818')
ax_right = fig.add_axes([0.53, 0.28, 0.44, 0.62], facecolor='#181818')

# Title Header
ax_title = fig.add_axes([0.05, 0.92, 0.90, 0.06], facecolor='none')
ax_title.axis('off')
title_text = ax_title.text(0.5, 0.5, 
                           'Interaktive Erklärung: Neuronale Netze & Gradientenabstieg (Aufgabe 3)', 
                           color='#00ffcc', fontsize=18, fontweight='bold', 
                           ha='center', va='center')

# Sliders styling
ax_color = '#252525'
slider_width = 0.36
slider_height = 0.022

# Left Column Sliders (for Forward Pass task 3a)
ax_x1 = fig.add_axes([0.08, 0.17, slider_width, slider_height], facecolor=ax_color)
ax_x2 = fig.add_axes([0.08, 0.12, slider_width, slider_height], facecolor=ax_color)

# Right Column Sliders (for Training task 3c)
ax_N = fig.add_axes([0.56, 0.17, slider_width, slider_height], facecolor=ax_color)
ax_B = fig.add_axes([0.56, 0.12, slider_width, slider_height], facecolor=ax_color)
ax_P = fig.add_axes([0.56, 0.07, slider_width, slider_height], facecolor=ax_color)
ax_E = fig.add_axes([0.08, 0.07, slider_width, slider_height], facecolor=ax_color) # placed on left for balance

# Define Sliders
s_x1 = Slider(ax_x1, 'Input x1', -5.0, 5.0, valinit=1.0, valfmt='%0.1f', color='#ff6666')
s_x2 = Slider(ax_x2, 'Input x2', -5.0, 5.0, valinit=3.0, valfmt='%0.1f', color='#ffcc66')

s_N = Slider(ax_N, 'Beispiele (N)', 100, 10000, valinit=4000, valfmt='%0.0f', color='#ff88ff')
s_B = Slider(ax_B, 'Minibatch (B)', 1, 1000, valinit=20, valfmt='%0.0f', color='#a366ff')
s_P = Slider(ax_P, 'Parameter (P)', 100, 5000, valinit=1500, valfmt='%0.0f', color='#66ccff')
s_E = Slider(ax_E, 'Epochen (E)', 1, 50, valinit=10, valfmt='%0.0f', color='#66ff99')

def draw_neural_network(ax, x1, x2):
    ax.clear()
    ax.set_xlim(-1, 11)
    ax.set_ylim(-1, 11)
    ax.axis('off')
    ax.set_title("Teil (a): Forward Pass & Begriffserklärung (Netzwerk)", color='#00ffcc', fontsize=12, pad=10)

    # 1. Forward Pass Calculations
    z_h1 = 2 * x1 + 1 * x2 + 0
    a_h1 = max(0, z_h1)  # ReLU
    
    z_h2 = -1 * x1 + 1 * x2 - 2
    a_h2 = max(0, z_h2)  # ReLU
    
    z_o = 1 * a_h1 - 2 * a_h2 + 1  # No activation
    a_o = z_o

    # 2. Draw Neurons (Circles)
    r_node = 0.65
    
    # Input Nodes (x1, x2)
    c_x1 = patches.Circle((1.5, 7.5), r_node, edgecolor='#ff6666', facecolor='#ff6666', alpha=0.2, lw=2)
    c_x2 = patches.Circle((1.5, 3.5), r_node, edgecolor='#ffcc66', facecolor='#ffcc66', alpha=0.2, lw=2)
    ax.add_patch(c_x1)
    ax.add_patch(c_x2)
    ax.text(1.5, 7.5, f"$x_1$\n{x1:.1f}", color='#ffffff', ha='center', va='center', fontsize=10, fontweight='bold')
    ax.text(1.5, 3.5, f"$x_2$\n{x2:.1f}", color='#ffffff', ha='center', va='center', fontsize=10, fontweight='bold')
    
    # Label for Inputs
    ax.text(1.5, 9.2, "Eingänge\n(Features)", color='#ffbb66', ha='center', va='center', fontsize=9, fontweight='bold')
    ax.text(1.5, 2.0, "Hier: D = 2\nFeatures", color='#ffbb66', ha='center', va='center', fontsize=8, style='italic')

    # Hidden Nodes (h1, h2)
    c_h1 = patches.Circle((5.5, 7.5), r_node, edgecolor='#a366ff', facecolor='#a366ff', alpha=0.2, lw=2)
    c_h2 = patches.Circle((5.5, 3.5), r_node, edgecolor='#a366ff', facecolor='#a366ff', alpha=0.2, lw=2)
    ax.add_patch(c_h1)
    ax.add_patch(c_h2)
    ax.text(5.5, 7.5, f"$h_1$\n{a_h1:.1f}", color='#ffffff', ha='center', va='center', fontsize=10, fontweight='bold')
    ax.text(5.5, 3.5, f"$h_2$\n{a_h2:.1f}", color='#ffffff', ha='center', va='center', fontsize=10, fontweight='bold')
    
    # Label for Hidden Layer
    ax.text(5.5, 9.2, "Verdeckte Schicht\n(Hidden Layer)", color='#c299ff', ha='center', va='center', fontsize=9, fontweight='bold')
    ax.text(5.5, 8.4, f"Bias: b = 0\nReLU(z) = max(0, z)\nIn: {z_h1:.1f} → Out: {a_h1:.1f}", color='#c299ff', ha='center', va='center', fontsize=7.5)
    ax.text(5.5, 2.5, f"Bias: b = -2\nReLU(z) = max(0, z)\nIn: {z_h2:.1f} → Out: {a_h2:.1f}", color='#c299ff', ha='center', va='center', fontsize=7.5)

    # Output Node (o)
    c_o = patches.Circle((9.5, 5.5), r_node, edgecolor='#00ffaa', facecolor='#00ffaa', alpha=0.2, lw=2)
    ax.add_patch(c_o)
    ax.text(9.5, 5.5, f"$o$\n{a_o:.1f}", color='#ffffff', ha='center', va='center', fontsize=11, fontweight='bold')
    
    # Label for Output Layer
    ax.text(9.5, 9.2, "Ausgabeschicht\n(Output)", color='#00ffaa', ha='center', va='center', fontsize=9, fontweight='bold')
    ax.text(9.5, 6.4, f"Bias: b = +1\nKeine Aktivierung\nAusgabe: {a_o:.1f}", color='#00ffaa', ha='center', va='center', fontsize=7.5)

    # 3. Draw Connections (Arrows) & Weights
    # Helper to draw annotated arrow
    def draw_arrow(start, end, label, label_pos, color='#ffffff'):
        ax.annotate('', xy=end, xytext=start,
                    arrowprops=dict(arrowstyle="->", color=color, lw=1.5, shrinkA=5, shrinkB=5))
        ax.text(label_pos[0], label_pos[1], label, color=color, fontsize=10, fontweight='bold', ha='center', va='center',
                bbox=dict(boxstyle="circle,pad=0.2", fc='#121212', ec=color, lw=1))

    # x1 -> h1
    draw_arrow((1.5, 7.5), (5.5, 7.5), "2", (3.5, 7.8), '#66ccff')
    # x1 -> h2
    draw_arrow((1.5, 7.5), (5.5, 3.5), "-1", (3.0, 5.8), '#66ccff')
    # x2 -> h1
    draw_arrow((1.5, 3.5), (5.5, 7.5), "1", (3.0, 5.2), '#66ccff')
    # x2 -> h2
    draw_arrow((1.5, 3.5), (5.5, 3.5), "1", (3.5, 3.2), '#66ccff')
    
    # h1 -> o
    draw_arrow((5.5, 7.5), (9.5, 5.5), "+1", (7.5, 6.8), '#66ccff')
    # h2 -> o
    draw_arrow((5.5, 3.5), (9.5, 5.5), "-2", (7.5, 4.2), '#66ccff')

    # Label for Weights
    ax.text(3.5, 9.2, "Gewichte (Weights)\nan den Pfeilen", color='#66ccff', ha='center', va='center', fontsize=9, fontweight='bold')
    
    # 4. Explanations Callout Box at the bottom left
    explanation_box = patches.FancyBboxPatch((0.2, 0.0), 10.1, 1.2, boxstyle="round,pad=0.1", 
                                             linewidth=1.0, edgecolor='#888888', facecolor='#222222')
    ax.add_patch(explanation_box)
    
    expl_text = (
        r"$\mathbf{Definitionen:}$" "\n"
        r"• $\mathbf{Gewicht\ (w):}$ Multipliziert das Eingangssignal an der Verbindung (z. B. $2$ von $x_1 \to h_1$)." "\n"
        r"• $\mathbf{Bias\ (b):}$ Ein additiver Wert direkt am Neuron (z. B. $b = -2$ an $h_2$). Verschiebt die Aktivierungsfunktion." "\n"
        r"• $\mathbf{Parameter\ (\theta):}$ Die Gesamtheit aller Gewichte & Biases. In diesem Netz: $\mathbf{6\ Gewichte + 3\ Biases = 9\ Parameter}$."
    )
    ax.text(0.4, 0.6, expl_text, color='#e0e0e0', ha='left', va='center', fontsize=8.5)


def draw_training_concepts(ax, N, B, P, E):
    ax.clear()
    ax.set_xlim(-1, 11)
    ax.set_ylim(-1, 11)
    ax.axis('off')
    ax.set_title("Teil (c): Logische Verbindungen beim Training", color='#ff88ff', fontsize=12, pad=10)

    # 1. Calculations
    loss_summands = N
    grad_dim = P
    batches_per_epoch = math.ceil(N / B)
    total_updates_minibatch = batches_per_epoch * E
    total_updates_batch = 1 * E

    # 2. Box showing (i) Summanden der Verlustfunktion
    rect_loss = patches.FancyBboxPatch((0.2, 7.8), 10.1, 2.0, boxstyle="round,pad=0.1", 
                                       linewidth=1.2, edgecolor='#ff6666', facecolor='#2d1616')
    ax.add_patch(rect_loss)
    
    text_i = (
        f"(i) Summanden der Verlustfunktion:\n"
        f"• Datensatz hat N = {N} Beispiele. Die Verlustfunktion summiert die Fehler jedes Beispiels:\n"
        f"  L(θ) = 1/{N} * [ L_1(θ) + L_2(θ) + ... + L_N(θ) ]\n"
        f"  → Anzahl der Summanden = N = {loss_summands} (einer pro Beispiel)"
    )
    ax.text(0.5, 8.8, text_i, color='#ffbbbb', ha='left', va='center', fontsize=9, fontweight='semibold')

    # 3. Box showing (ii) & (iii) Dimension des Gradienten
    rect_grad = patches.FancyBboxPatch((0.2, 4.2), 10.1, 3.2, boxstyle="round,pad=0.1", 
                                       linewidth=1.2, edgecolor='#66ccff', facecolor='#16252d')
    ax.add_patch(rect_grad)
    
    text_ii_iii = (
        f"(ii) & (iii) Dimension des Gradienten:\n"
        f"• Der Gradient ∇L enthält die Ableitungen nach allen Parametern θ (Gewichte + Biases).\n"
        f"• Da das Netz P = {P} Parameter hat, ist der Gradient immer ein P-dimensionaler Vektor:\n"
        f"  dim(∇L) = P = {grad_dim}\n"
        f"• Das gilt sowohl für die gesamte Verlustfunktion als auch für eine kleine Minibatch (B = {B})!\n"
        f"  (Die Batchgröße ändert nicht die Anzahl der Stellschrauben im Netz!)\n"
        f"  → Dimension des gesamten Gradienten = {grad_dim}\n"
        f"  → Dimension des Minibatch-Gradienten = {grad_dim}"
    )
    ax.text(0.5, 5.8, text_ii_iii, color='#bbddff', ha='left', va='center', fontsize=9, fontweight='semibold')

    # 4. Box showing (iv) & (v) Updates
    rect_updates = patches.FancyBboxPatch((0.2, 0.0), 10.1, 3.8, boxstyle="round,pad=0.1", 
                                          linewidth=1.2, edgecolor='#a366ff', facecolor='#20162d')
    ax.add_patch(rect_updates)
    
    text_iv_v = (
        f"(iv) & (v) Anzahl der Parameter-Updates (Epochen E = {E}):\n"
        f"• Minibatch-SGD (Batchgröße B = {B}):\n"
        f"  - Batches pro Epoche = ⌈N / B⌉ = ⌈{N} / {B}⌉ = {batches_per_epoch} Batches\n"
        f"  - Pro Batch erfolgt 1 Gewichtsupdate (Δθ)\n"
        f"  - Updates in {E} Epochen = {batches_per_epoch} * {E} = {total_updates_minibatch} Updates\n\n"
        f"• Batch-Gradientenabstieg (Batchgröße B = N = {N}):\n"
        f"  - Batch pro Epoche = 1 (der gesamte Datensatz auf einmal)\n"
        f"  - Updates in {E} Epochen = 1 * {E} = {total_updates_batch} Updates"
    )
    ax.text(0.5, 1.9, text_iv_v, color='#e0bbff', ha='left', va='center', fontsize=9, fontweight='semibold')


def update(val):
    x1 = s_x1.val
    x2 = s_x2.val
    N = int(s_N.val)
    B = int(s_B.val)
    P = int(s_P.val)
    E = int(s_E.val)

    draw_neural_network(ax_left, x1, x2)
    draw_training_concepts(ax_right, N, B, P, E)
    fig.canvas.draw_idle()


# Hook update function to all sliders
s_x1.on_changed(update)
s_x2.on_changed(update)
s_N.on_changed(update)
s_B.on_changed(update)
s_P.on_changed(update)
s_E.on_changed(update)

# Initial draw
update(None)

# Show layout
plt.show()
