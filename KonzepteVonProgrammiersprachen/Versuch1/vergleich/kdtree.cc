#include "kdtree.h"
#include <algorithm>
#include <limits>

// ---------------------------------------------------------------------------
// Hilfsfunktionen (nur intern)
// ---------------------------------------------------------------------------

static float centroid_axis(const Triangle& t, int axis) {
    if (axis == 0) return (t.v0.x + t.v1.x + t.v2.x) * (1.f / 3.f);
    else if (axis == 1) return (t.v0.y + t.v1.y + t.v2.y) * (1.f / 3.f);
    else               return (t.v0.z + t.v1.z + t.v2.z) * (1.f / 3.f);
}

// ---------------------------------------------------------------------------
// Rekursiver Build (arbeitet in-place auf ordered[begin..end))
// ---------------------------------------------------------------------------

static int build_rec(std::vector<KDNode>&       nodes,
                     std::vector<int>&           ordered,
                     const std::vector<Triangle>& tris,
                     int begin, int end, int depth)
{
    KDNode node;

    // AABB aller Dreiecke in diesem Bereich berechnen
    for (int i = begin; i < end; ++i) {
        node.bounds.expand(tris[ordered[i]].v0);
        node.bounds.expand(tris[ordered[i]].v1);
        node.bounds.expand(tris[ordered[i]].v2);
    }

    int count = end - begin;

    // Blattknoten: zu wenige Dreiecke oder maximale Tiefe erreicht
    if (count <= KD_LEAF_TRIS || depth >= KD_MAX_DEPTH) {
        node.tri_begin = begin;
        node.tri_end   = end;
        int idx = (int)nodes.size();
        nodes.push_back(node);
        return idx;
    }

    // Längste Achse der AABB als Trennachse wählen
    Vec3 extent = node.bounds.max - node.bounds.min;
    if      (extent.x >= extent.y && extent.x >= extent.z) node.axis = 0;
    else if (extent.y >= extent.z)                          node.axis = 1;
    else                                                    node.axis = 2;

    int ax  = node.axis;
    int mid = begin + count / 2;

    // Medianaufteilung: nth_element O(n) reicht aus (kein vollständiges Sort nötig)
    std::nth_element(ordered.begin() + begin,
                     ordered.begin() + mid,
                     ordered.begin() + end,
                     [&](int a, int b) {
                         return centroid_axis(tris[a], ax) <
                                centroid_axis(tris[b], ax);
                     });

    // Platzhalter einfügen; da nodes.reserve() aufgerufen wurde, bleibt
    // der Zeiger stabil – nodes[idx] ist nach den rekursiven Aufrufen noch gültig.
    int idx = (int)nodes.size();
    nodes.push_back(node);

    nodes[idx].left  = build_rec(nodes, ordered, tris, begin, mid, depth + 1);
    nodes[idx].right = build_rec(nodes, ordered, tris, mid,   end, depth + 1);
    return idx;
}

// ---------------------------------------------------------------------------
// Öffentliche Build-Funktion
// ---------------------------------------------------------------------------

void KDTree::build(const std::vector<Triangle>& triangles)
{
    int n = (int)triangles.size();
    ordered_tris.resize(n);
    for (int i = 0; i < n; ++i) ordered_tris[i] = i;

    nodes.clear();
    // Obere Schranke: vollständiger Binärbaum mit n/KD_LEAF_TRIS Blättern
    nodes.reserve(2 * n + 8);

    if (n > 0)
        build_rec(nodes, ordered_tris, triangles, 0, n, 0);
}

// ---------------------------------------------------------------------------
// Traversal: nächster Treffer
// ---------------------------------------------------------------------------

int KDTree::closest_hit(const std::vector<Triangle>& tris, const Ray& ray,
                        float tmin, float tmax,
                        float& out_t, float& out_u, float& out_v) const
{
    if (nodes.empty()) return -1;
    if (!nodes[0].bounds.intersect(ray, tmin, tmax)) return -1;

    int   best   = -1;
    float best_t = tmax;

    // Stack-basierte Traversal (Tiefe ≤ KD_MAX_DEPTH, 64 Einträge reichen immer)
    int stack[64];
    int top = 0;
    stack[top++] = 0;

    while (top > 0) {
        const KDNode& node = nodes[stack[--top]];

        // Knoten überspringen, wenn AABB weiter als bisheriger Treffer
        if (!node.bounds.intersect(ray, tmin, best_t)) continue;

        if (node.is_leaf()) {
            float u, v;
            for (int i = node.tri_begin; i < node.tri_end; ++i) {
                float t = tris[ordered_tris[i]].intersect(ray, tmin, best_t, u, v);
                if (t > 0.f) {
                    best   = ordered_tris[i];
                    best_t = t;
                    out_u  = u;
                    out_v  = v;
                }
            }
        } else {
            stack[top++] = node.left;
            stack[top++] = node.right;
        }
    }

    out_t = best_t;
    return best;
}

// ---------------------------------------------------------------------------
// Traversal: Schattentest (frühzeitiger Abbruch bei erstem Treffer)
// ---------------------------------------------------------------------------

bool KDTree::any_hit(const std::vector<Triangle>& tris, const Ray& ray,
                     float tmin, float tmax) const
{
    if (nodes.empty()) return false;
    if (!nodes[0].bounds.intersect(ray, tmin, tmax)) return false;

    int stack[64];
    int top = 0;
    stack[top++] = 0;

    while (top > 0) {
        const KDNode& node = nodes[stack[--top]];

        if (!node.bounds.intersect(ray, tmin, tmax)) continue;

        if (node.is_leaf()) {
            float u, v;
            for (int i = node.tri_begin; i < node.tri_end; ++i) {
                if (tris[ordered_tris[i]].intersect(ray, tmin, tmax, u, v) > 0.f)
                    return true;
            }
        } else {
            stack[top++] = node.left;
            stack[top++] = node.right;
        }
    }
    return false;
}

// =============================================================================
// AVX2 Packet Tracing (8 Strahlen gleichzeitig)
// <immintrin.h> wird nur hier eingebunden, nicht transitiv über raypacket.h.
// =============================================================================
#ifdef HAVE_AVX2
#include <immintrin.h>

// ---------------------------------------------------------------------------
// Slab-Test für 8 Strahlen gegen eine AABB (AVX2).
// Gibt Bitmask zurück: Bit i gesetzt = Strahl i trifft die AABB.
// ---------------------------------------------------------------------------
static int slab_test_avx(const RayPacket& p, const AABB& b,
                          __m256 tmin, __m256 best_t)
{
    // Paket-Komponenten aus float[8]-Arrays laden
    __m256 ox  = _mm256_load_ps(p.ox),  oy  = _mm256_load_ps(p.oy),  oz  = _mm256_load_ps(p.oz);
    __m256 idx = _mm256_load_ps(p.idx), idy = _mm256_load_ps(p.idy), idz = _mm256_load_ps(p.idz);

    auto slab = [](float bmin, float bmax, __m256 o, __m256 id) {
        __m256 t1 = _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(bmin), o), id);
        __m256 t2 = _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(bmax), o), id);
        return std::make_pair(_mm256_min_ps(t1, t2), _mm256_max_ps(t1, t2));
    };

    auto [tex, tfx] = slab(b.min.x, b.max.x, ox, idx);
    auto [tey, tfy] = slab(b.min.y, b.max.y, oy, idy);
    auto [tez, tfz] = slab(b.min.z, b.max.z, oz, idz);

    __m256 te = _mm256_max_ps(tex, _mm256_max_ps(tey, tez));
    __m256 tf = _mm256_min_ps(tfx, _mm256_min_ps(tfy, tfz));

    // Treffer: tenter ≤ texit  UND  texit ≥ tmin  UND  tenter < best_t
    __m256 hit = _mm256_and_ps(
                    _mm256_cmp_ps(te, tf,     _CMP_LE_OQ),
                 _mm256_and_ps(
                    _mm256_cmp_ps(tf, tmin,   _CMP_GE_OQ),
                    _mm256_cmp_ps(te, best_t, _CMP_LT_OQ)));
    return _mm256_movemask_ps(hit);
}

// ---------------------------------------------------------------------------
// Möller-Trumbore für 8 Strahlen gegen 1 Dreieck (AVX2 + FMA).
// Aktualisiert best_t, out_u, out_v; gibt Treffer-Bitmask zurück.
// ---------------------------------------------------------------------------
static int mt_avx(const RayPacket& p, const Triangle& tri,
                  __m256 tmin, __m256& best_t,
                  float* __restrict__ out_u, float* __restrict__ out_v)
{
    // Paket-Komponenten laden
    __m256 ox = _mm256_load_ps(p.ox), oy = _mm256_load_ps(p.oy), oz = _mm256_load_ps(p.oz);
    __m256 dx = _mm256_load_ps(p.dx), dy = _mm256_load_ps(p.dy), dz = _mm256_load_ps(p.dz);

    __m256 e1x = _mm256_set1_ps(tri.v1.x - tri.v0.x);
    __m256 e1y = _mm256_set1_ps(tri.v1.y - tri.v0.y);
    __m256 e1z = _mm256_set1_ps(tri.v1.z - tri.v0.z);
    __m256 e2x = _mm256_set1_ps(tri.v2.x - tri.v0.x);
    __m256 e2y = _mm256_set1_ps(tri.v2.y - tri.v0.y);
    __m256 e2z = _mm256_set1_ps(tri.v2.z - tri.v0.z);

    // h = d × e2  (FMA: a*b - c)
    __m256 hx = _mm256_fmsub_ps(dy, e2z, _mm256_mul_ps(dz, e2y));
    __m256 hy = _mm256_fmsub_ps(dz, e2x, _mm256_mul_ps(dx, e2z));
    __m256 hz = _mm256_fmsub_ps(dx, e2y, _mm256_mul_ps(dy, e2x));

    // det = e1 · h
    __m256 det = _mm256_fmadd_ps(e1x, hx,
                    _mm256_fmadd_ps(e1y, hy, _mm256_mul_ps(e1z, hz)));

    __m256 abs_det = _mm256_andnot_ps(_mm256_set1_ps(-0.f), det);
    __m256 valid   = _mm256_cmp_ps(abs_det, _mm256_set1_ps(MT_EPS), _CMP_GE_OQ);
    if (!_mm256_movemask_ps(valid)) return 0;

    __m256 inv = _mm256_div_ps(_mm256_set1_ps(1.f), det);
    __m256 sx  = _mm256_sub_ps(ox, _mm256_set1_ps(tri.v0.x));
    __m256 sy  = _mm256_sub_ps(oy, _mm256_set1_ps(tri.v0.y));
    __m256 sz  = _mm256_sub_ps(oz, _mm256_set1_ps(tri.v0.z));

    // u = inv * (s · h)
    __m256 u = _mm256_mul_ps(inv, _mm256_fmadd_ps(sx, hx,
                                      _mm256_fmadd_ps(sy, hy, _mm256_mul_ps(sz, hz))));
    valid = _mm256_and_ps(valid, _mm256_cmp_ps(u, _mm256_setzero_ps(), _CMP_GE_OQ));
    valid = _mm256_and_ps(valid, _mm256_cmp_ps(u, _mm256_set1_ps(1.f),  _CMP_LE_OQ));
    if (!_mm256_movemask_ps(valid)) return 0;

    // q = s × e1
    __m256 qx = _mm256_fmsub_ps(sy, e1z, _mm256_mul_ps(sz, e1y));
    __m256 qy = _mm256_fmsub_ps(sz, e1x, _mm256_mul_ps(sx, e1z));
    __m256 qz = _mm256_fmsub_ps(sx, e1y, _mm256_mul_ps(sy, e1x));

    // v = inv * (d · q)
    __m256 v = _mm256_mul_ps(inv, _mm256_fmadd_ps(dx, qx,
                                      _mm256_fmadd_ps(dy, qy, _mm256_mul_ps(dz, qz))));
    valid = _mm256_and_ps(valid, _mm256_cmp_ps(v, _mm256_setzero_ps(), _CMP_GE_OQ));
    valid = _mm256_and_ps(valid, _mm256_cmp_ps(_mm256_add_ps(u, v),
                                               _mm256_set1_ps(1.f), _CMP_LE_OQ));
    if (!_mm256_movemask_ps(valid)) return 0;

    // t = inv * (e2 · q)
    __m256 t = _mm256_mul_ps(inv, _mm256_fmadd_ps(e2x, qx,
                                      _mm256_fmadd_ps(e2y, qy, _mm256_mul_ps(e2z, qz))));
    valid = _mm256_and_ps(valid, _mm256_cmp_ps(t, tmin,   _CMP_GE_OQ));
    valid = _mm256_and_ps(valid, _mm256_cmp_ps(t, best_t, _CMP_LT_OQ));

    int mask = _mm256_movemask_ps(valid);
    if (!mask) return 0;

    best_t = _mm256_blendv_ps(best_t, t, valid);
    _mm256_storeu_ps(out_u, _mm256_blendv_ps(_mm256_loadu_ps(out_u), u, valid));
    _mm256_storeu_ps(out_v, _mm256_blendv_ps(_mm256_loadu_ps(out_v), v, valid));
    return mask;
}

// ---------------------------------------------------------------------------
// Packet-Traversal: nächster Treffer für 8 Strahlen gleichzeitig
// ---------------------------------------------------------------------------

void KDTree::closest_hit_packet(const std::vector<Triangle>& tris,
                                 const RayPacket& p, float tmin_f,
                                 float out_t[8], float out_u[8], float out_v[8],
                                 int   out_tri[8]) const
{
    if (nodes.empty()) return;

    __m256 tmin   = _mm256_set1_ps(tmin_f);
    __m256 best_t = _mm256_loadu_ps(out_t); // mit INF vorinitialisiert

    alignas(32) float best_u[8] = {};
    alignas(32) float best_v[8] = {};

    int stack[64];
    int top = 0;
    if (slab_test_avx(p, nodes[0].bounds, tmin, best_t))
        stack[top++] = 0;

    while (top > 0) {
        const KDNode& node = nodes[stack[--top]];
        if (!slab_test_avx(p, node.bounds, tmin, best_t)) continue;

        if (node.is_leaf()) {
            for (int i = node.tri_begin; i < node.tri_end; ++i) {
                int tid  = ordered_tris[i];
                int mask = mt_avx(p, tris[tid], tmin, best_t, best_u, best_v);
                for (int lane = 0; lane < 8; ++lane)
                    if (mask & (1 << lane)) out_tri[lane] = tid;
            }
        } else {
            stack[top++] = node.left;
            stack[top++] = node.right;
        }
    }

    _mm256_storeu_ps(out_t, best_t);
    _mm256_storeu_ps(out_u, _mm256_load_ps(best_u));
    _mm256_storeu_ps(out_v, _mm256_load_ps(best_v));
}
#endif // HAVE_AVX2
