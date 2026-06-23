#pragma once

#include <experimental/simd>
#include <glm/glm.hpp>
#include <limits>

namespace stdx = std::experimental;

using floatv  = stdx::native_simd<float>;
using intv    = stdx::native_simd<int>;
using maskv   = stdx::native_simd_mask<float>;
using imaskv  = stdx::native_simd_mask<int>;

struct RayPacket {
    floatv ox, oy, oz;
    floatv dx, dy, dz;
    floatv invDx, invDy, invDz;
};

struct IntersectionPacket {
    maskv hit = maskv(false);
    floatv t = floatv(std::numeric_limits<float>::max());
    floatv u = floatv(0.0f);
    floatv v = floatv(0.0f);
    floatv triIndex = floatv(-1.0f); // Index of the hit triangle in the mesh, -1 if no hit
};
