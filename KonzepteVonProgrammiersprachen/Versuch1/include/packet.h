#pragma once

#include <experimental/simd>
#include <glm/glm.hpp>
#include <limits>

namespace stdx = std::experimental;

// Definition der SIMD-Typen (unter Verwendung des nativen Vektorregisters, z.B. AVX2 = 8 floats)
using floatv  = stdx::native_simd<float>;
using intv    = stdx::native_simd<int>;
using maskv   = stdx::native_simd_mask<float>;
using imaskv  = stdx::native_simd_mask<int>;

// Struktur für ein Strahlenpaket im SoA-Format (Structure of Arrays)
struct RayPacket {
    floatv ox, oy, oz;       // Ursprünge der Strahlen (X, Y, Z)
    floatv dx, dy, dz;       // Richtungsvektoren der Strahlen (X, Y, Z)
    floatv invDx, invDy, invDz; // Invertierte Richtungen für den optimierten Slab-Test
};

// Struktur für die Treffer-Ergebnisse eines gesamten Strahlenpakets
struct IntersectionPacket {
    maskv hit = maskv(false); // Wahr, wenn der Strahl mindestens ein Dreieck getroffen hat
    floatv t = floatv(std::numeric_limits<float>::max()); // Distanz zum Schnittpunkt
    floatv u = floatv(0.0f);  // Baryzentrische Koordinate u
    floatv v = floatv(0.0f);  // Baryzentrische Koordinate v
    floatv triIndex = floatv(-1.0f); // Index des getroffenen Dreiecks im Mesh (als floatv für Vermeidung von Masken-Casts)
};
