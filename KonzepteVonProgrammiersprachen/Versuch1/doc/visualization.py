import sys
import subprocess

def install_and_import(package):
    try:
        __import__(package)
    except ImportError:
        subprocess.check_call([sys.executable, "-m", "pip", "install", package])

install_and_import('matplotlib')
install_and_import('numpy')

import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import numpy as np
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

# Setup Daten
v0 = np.array([1.0, 1.0, 1.0])
<<<<<<< HEAD
v1 = np.array([3.0, 1.0, 1.0])
=======
v1 = np.array([3.0, 1.0, 3.0])
>>>>>>> aed34e1b547cd5a6769859a49b1aebb7ae3de25a
v2 = np.array([1.0, 3.0, 1.0])

ray_origin = np.array([1.5, 1.5, 4.0])
ray_direction = np.array([0.0, 0.0, -1.0])

# Vorausberechnungen
edge1 = v1 - v0
edge2 = v2 - v0
h = np.cross(ray_direction, edge2)
a = np.dot(edge1, h)

f = 1.0 / a
s = ray_origin - v0
u = f * np.dot(s, h)

q = np.cross(s, edge1)
v = f * np.dot(ray_direction, q)

t = f * np.dot(edge2, q)
intersection = ray_origin + ray_direction * t

current_step = 0
max_step = 5

fig = plt.figure(figsize=(12, 9))
ax = fig.add_subplot(111, projection='3d')

def draw_step(step):
    ax.clear()
<<<<<<< HEAD
    
=======

>>>>>>> aed34e1b547cd5a6769859a49b1aebb7ae3de25a
    # Statische Elemente (immer sichtbar)
    verts = [[v0, v1, v2]]
    poly = Poly3DCollection(verts, alpha=0.2, facecolor='cyan', edgecolor='blue')
    ax.add_collection3d(poly)
<<<<<<< HEAD
    
=======

>>>>>>> aed34e1b547cd5a6769859a49b1aebb7ae3de25a
    ax.scatter([v0[0], v1[0], v2[0]], [v0[1], v1[1], v2[1]], [v0[2], v1[2], v2[2]], color='blue', s=20)
    ax.text(v0[0], v0[1], v0[2], ' v0', color='blue')
    ax.text(v1[0], v1[1], v1[2], ' v1', color='blue')
    ax.text(v2[0], v2[1], v2[2], ' v2', color='blue')
<<<<<<< HEAD
    
=======

>>>>>>> aed34e1b547cd5a6769859a49b1aebb7ae3de25a
    ray_end = ray_origin + ray_direction * 4
    ax.plot([ray_origin[0], ray_end[0]], [ray_origin[1], ray_end[1]], [ray_origin[2], ray_end[2]], color='red', alpha=0.5)
    ax.scatter(*ray_origin, color='red', s=50, marker='s')
    ax.text(ray_origin[0], ray_origin[1], ray_origin[2], ' Ray Origin', color='red')

    title = "Schritt 0: Ausgangslage\nKamera(Strahl) und Dreieck sind im Raum"
<<<<<<< HEAD
    
=======

>>>>>>> aed34e1b547cd5a6769859a49b1aebb7ae3de25a
    # Step 1: Kanten
    if step >= 1:
        ax.quiver(v0[0], v0[1], v0[2], edge1[0], edge1[1], edge1[2], color='green', label='edge1', arrow_length_ratio=0.1)
        ax.quiver(v0[0], v0[1], v0[2], edge2[0], edge2[1], edge2[2], color='purple', label='edge2', arrow_length_ratio=0.1)
        title = "Schritt 1 (Zeile 11-13): Kanten berechnen\nedge1 = v1-v0  |  edge2 = v2-v0"

    # Step 2: h und a
    if step >= 2:
        h_scaled = h * 0.5
        ax.quiver(v0[0], v0[1], v0[2], h_scaled[0], h_scaled[1], h_scaled[2], color='cyan', label='Vektor h (dir x edge2)', arrow_length_ratio=0.1)
        title = f"Schritt 2 (Zeile 15-26): Parallel-Test\nOrthogonaler Vektor h = ray.dir x edge2\na = dot(edge1, h) = {a:.2f}"

    # Step 3: s und u
    if step >= 3:
        ax.quiver(v0[0], v0[1], v0[2], s[0], s[1], s[2], color='gray', linestyle='dashed', label='Vektor s (origin-v0)', arrow_length_ratio=0.1)
        u_vec = u * edge1
        ax.quiver(v0[0], v0[1], v0[2], u_vec[0], u_vec[1], u_vec[2], color='lime', linewidth=4, label=f'u * edge1 (u={u:.2f})', arrow_length_ratio=0.1)
        title = f"Schritt 3 (Zeile 28-35): Vektor s & Parameter u\nAbstand s = ray.origin - v0\nu = f * dot(s, h) = {u:.2f}"

    # Step 4: q und v
    if step >= 4:
        q_scaled = q * 0.2
        ax.quiver(v0[0], v0[1], v0[2], q_scaled[0], q_scaled[1], q_scaled[2], color='magenta', label='Vektor q (s x edge1)', arrow_length_ratio=0.1)
        u_vec = u * edge1
        v_vec = v * edge2
        end_of_u = v0 + u_vec
        ax.quiver(end_of_u[0], end_of_u[1], end_of_u[2], v_vec[0], v_vec[1], v_vec[2], color='violet', linewidth=4, label=f'v * edge2 (v={v:.2f})', arrow_length_ratio=0.1)
        title = f"Schritt 4 (Zeile 37-43): Vektor q & Parameter v\nq = s x edge1\nv = f * dot(ray.dir, q) = {v:.2f}"

    # Step 5: t / Schnittpunkt
    if step >= 5:
        ax.scatter(*intersection, color='orange', s=200, marker='*', label='Schnittpunkt')
        title = f"Schritt 5 (Zeile 45-53): Entfernung t / Treffer\nt = f * dot(edge2, q) = {t:.2f}\nSchnittpunkt ist gefunden!"

    ax.set_title(title, fontsize=12, fontweight='bold')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_xlim([0, 4])
    ax.set_ylim([-1, 4])
    ax.set_zlim([0, 5])
<<<<<<< HEAD
    
    if step > 0:
        ax.legend(loc='center left', bbox_to_anchor=(1.05, 0.5))
    
=======

    if step > 0:
        ax.legend(loc='center left', bbox_to_anchor=(1.05, 0.5))

>>>>>>> aed34e1b547cd5a6769859a49b1aebb7ae3de25a
    plt.subplots_adjust(right=0.75, bottom=0.2)
    fig.canvas.draw_idle()

draw_step(current_step)

axprev = fig.add_axes([0.3, 0.05, 0.15, 0.075])
axnext = fig.add_axes([0.55, 0.05, 0.15, 0.075])
bnext = Button(axnext, 'Nächster Schritt ->')
bprev = Button(axprev, '<- Zurück')

def next_step(event):
    global current_step
    if current_step < max_step:
        current_step += 1
        draw_step(current_step)

def prev_step(event):
    global current_step
    if current_step > 0:
        current_step -= 1
        draw_step(current_step)

bnext.on_clicked(next_step)
bprev.on_clicked(prev_step)

plt.show()
