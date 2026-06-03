import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

def visualize_slab_test():
    # 1. Definition der Bounding Box (Unser "Karton")
    # Min = unten links, Max = oben rechts
    box_min = np.array([2.0, 2.0])
    box_max = np.array([6.0, 5.0])

    # 2. Definition des Strahls (Unser "Laserpointer")
    ray_origin = np.array([0.0, 1.0])
    ray_direction = np.array([1.5, 1.0]) # Richtung (X, Y)
    
    # Richtung normalisieren (auf Länge 1 bringen für korrekte Entfernungsberechnung)
    ray_direction = ray_direction / np.linalg.norm(ray_direction)

    # --- DER SLAB-TEST ---
    # Wir berechnen den Kehrwert der Richtung, um Division durch Null zu vermeiden
    # (für den Fall, dass der Strahl exakt waagerecht oder senkrecht verläuft)
    dir_frac = np.empty(2)
    dir_frac[0] = 1.0 / ray_direction[0] if ray_direction[0] != 0 else float('inf')
    dir_frac[1] = 1.0 / ray_direction[1] if ray_direction[1] != 0 else float('inf')

    # Berechne die Schnittpunkte mit den X-Slabs (linke und rechte unendliche Wand)
    t1 = (box_min[0] - ray_origin[0]) * dir_frac[0]
    t2 = (box_max[0] - ray_origin[0]) * dir_frac[0]
    
    # Berechne die Schnittpunkte mit den Y-Slabs (untere und obere unendliche Wand)
    t3 = (box_min[1] - ray_origin[1]) * dir_frac[1]
    t4 = (box_max[1] - ray_origin[1]) * dir_frac[1]

    # Finde die minimalen und maximalen Distanzen für jede Achse
    t_min_x = min(t1, t2)
    t_max_x = max(t1, t2)
    t_min_y = min(t3, t4)
    t_max_y = max(t3, t4)

    # t_near: Wann betritt der Strahl den überschneidenden Raum aller Slabs?
    t_near = max(t_min_x, t_min_y)
    
    # t_far: Wann verlässt der Strahl diesen Raum wieder?
    t_far = min(t_max_x, t_max_y)

    # Treffer-Bedingung: Der Eintrittspunkt muss VOR dem Austrittspunkt liegen 
    # und der Austrittspunkt muss VOR uns (positiv) liegen.
    hit = t_near <= t_far and t_far >= 0

    # --- VISUALISIERUNG ---
    fig, ax = plt.subplots(figsize=(8, 8))
    
    # Raster und Achsen einrichten
    ax.set_xlim(-1, 8)
    ax.set_ylim(-1, 8)
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.set_aspect('equal')

    # Zeichne den Karton (Bounding Box)
    rect = patches.Rectangle((box_min[0], box_min[1]), 
                             box_max[0] - box_min[0], 
                             box_max[1] - box_min[1], 
                             linewidth=2, edgecolor='green', facecolor='lightgreen', alpha=0.5, 
                             label='Karton (AABB)')
    ax.add_patch(rect)

    # Zeichne die Slabs (die unendlichen Wände)
    ax.axvline(x=box_min[0], color='gray', linestyle=':', label='X-Slabs (links/rechts)')
    ax.axvline(x=box_max[0], color='gray', linestyle=':')
    ax.axhline(y=box_min[1], color='orange', linestyle=':', label='Y-Slabs (unten/oben)')
    ax.axhline(y=box_max[1], color='orange', linestyle=':')

    # Zeichne den Strahl (Laser)
    ray_end = ray_origin + ray_direction * 10  # Mach den Strahl künstlich lang für die Anzeige
    ax.plot([ray_origin[0], ray_end[0]], [ray_origin[1], ray_end[1]], color='red', linewidth=1.5, label='Strahl')
    ax.plot(ray_origin[0], ray_origin[1], 'ro', label='Laser-Ursprung')

    # Zeichne das Ergebnis ein
    if hit:
        # Markiere die exakte Strecke, auf der der Strahl durch den Karton wandert
        hit_start = ray_origin + ray_direction * max(0, t_near)
        hit_end = ray_origin + ray_direction * t_far
        ax.plot([hit_start[0], hit_end[0]], [hit_start[1], hit_end[1]], color='blue', linewidth=4, label='Trefferstrecke!')
        ax.set_title("Slab-Test: TREFFER! (Strahl kreuzt die Box)", color='green', fontweight='bold', fontsize=14)
    else:
        ax.set_title("Slab-Test: DANEBEN! (Strahl verfehlt die Box)", color='red', fontweight='bold', fontsize=14)

    # Info-Box mit den berechneten Werten
    info_text = f"t_near (Eintritt): {t_near:.2f}\nt_far (Austritt): {t_far:.2f}"
    plt.text(-0.5, 7.0, info_text, bbox=dict(facecolor='white', alpha=0.8, edgecolor='black'))

    ax.legend(loc='lower right')
    plt.xlabel("X-Achse")
    plt.ylabel("Y-Achse")
    
    # Fenster anzeigen
    plt.show()

if __name__ == "__main__":
    visualize_slab_test()