#include "renderer.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>
#include <algorithm>

static constexpr float INF = std::numeric_limits<float>::max();

// Sucht nächstes Dreieck via k-d-Baum; gibt Index zurück oder -1.
static int closest_hit(const Scene& sc, const Ray& ray, float tmin,
                       float& out_t, float& out_u, float& out_v) {
    return sc.kd.closest_hit(sc.triangles, ray, tmin, INF, out_t, out_u, out_v);
}

// Schattentest via k-d-Baum: true bei erstem Treffer in [tmin, tmax].
static bool any_hit(const Scene& sc, const Ray& ray, float tmin, float tmax) {
    return sc.kd.any_hit(sc.triangles, ray, tmin, tmax);
}

// Phong-Beleuchtung am Treffpunkt.
static Vec3 shade(const Scene& sc, const Ray& ray,
                  int idx, float hit_t, float u, float v) {
    const Triangle& tri = sc.triangles[idx];
    int mat_id = std::min(tri.mat_id, (int)sc.materials.size() - 1);
    const Material& mat = sc.materials[mat_id];

    Vec3 P = ray.at(hit_t);
    Vec3 N = tri.shading_normal(u, v);
    if (N.dot(-ray.direction) < 0.f) N = -N; // Normale zur Kamera hin drehen
    Vec3 V = -ray.direction;                   // Richtung zur Kamera

    Vec3 color = mat.ambient;

    for (const PointLight& light : sc.lights) {
        Vec3  L_vec = light.position - P;
        float dist  = L_vec.length();
        Vec3  L     = L_vec / dist;

        // Schattenstrahl: von P zur Lichtquelle
        if (any_hit(sc, {P, L}, 1e-4f, dist - 1e-4f)) continue;

        float NdL  = std::max(0.f, N.dot(L));
        // Phong-Reflexionsvektor: R = 2*(N·L)*N - L
        Vec3  R    = N * (2.f * NdL) - L;
        float spec = (NdL > 0.f) ? std::pow(std::max(0.f, R.dot(V)), mat.shininess)
                                 : 0.f;

        Vec3 lc = light.color * light.intensity;
        color += mat.diffuse  * NdL  * lc;
        color += mat.specular * spec * lc;
    }
    return color;
}

void render_brute(const Scene& sc, const Camera& cam, Image& img, bool use_mt) {
    int W = img.width, H = img.height;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            float s = (x + 0.5f) / W;
            float t = (y + 0.5f) / H;
            Ray ray = cam.get_ray(s, t);

            int   best_idx = -1;
            float best_t   = INF, best_u = 0.f, best_v = 0.f;

            for (int i = 0; i < (int)sc.triangles.size(); ++i) {
                float u, v, hit;
                if (use_mt)
                    hit = sc.triangles[i].intersect(ray, 1e-4f, best_t, u, v);
                else
                    hit = sc.triangles[i].intersect_naive(ray, 1e-4f, best_t, u, v);
                if (hit > 0.f) { best_idx = i; best_t = hit; best_u = u; best_v = v; }
            }

            Vec3 col = (best_idx >= 0) ? shade(sc, ray, best_idx, best_t, best_u, best_v)
                                       : sc.background;
            img.set(x, H - 1 - y, col);
        }

        if ((y + 1) % 50 == 0 || y + 1 == H)
            std::cout << "\rRendere (Brute-Force)... " << (y + 1) << "/" << H << std::flush;
    }
    std::cout << "\n";
}

void render(const Scene& sc, const Camera& cam, Image& img) {
    int W = img.width, H = img.height;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            float s = (x + 0.5f) / W;
            float t = (y + 0.5f) / H;
            Ray ray = cam.get_ray(s, t);

            float hit_t, u, v;
            int idx = closest_hit(sc, ray, 1e-4f, hit_t, u, v);

            Vec3 col = (idx >= 0) ? shade(sc, ray, idx, hit_t, u, v)
                                  : sc.background;
            // Y-Achse: PPM hat Ursprung oben links, wir rechnen von unten
            img.set(x, H - 1 - y, col);
        }

        if ((y + 1) % 50 == 0 || y + 1 == H)
            std::cout << "\rRendere... " << (y + 1) << "/" << H << std::flush;
    }
    std::cout << "\n";
}

// =============================================================================
// AVX2 Packet Tracing (8 Primärstrahlen gleichzeitig)
// =============================================================================
#ifdef HAVE_AVX2

void render_packet(const Scene& sc, const Camera& cam, Image& img)
{
    int W = img.width, H = img.height;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; x += PACKET_SIZE) {
            // Anzahl gültiger Strahlen (letzte Gruppe kann < 8 sein)
            int count = std::min(PACKET_SIZE, W - x);

            // Strahlen generieren – inaktive Lanes erhalten Kopie der letzten gültigen Lane
            RayPacket pkt;
            for (int i = 0; i < PACKET_SIZE; ++i) {
                int xi = x + std::min(i, count - 1); // Padding: letzte Lane wiederholen
                float s = (xi + 0.5f) / W;
                float t = (y  + 0.5f) / H;
                Ray r   = cam.get_ray(s, t);
                pkt.ox[i]  = r.origin.x;     pkt.oy[i]  = r.origin.y;     pkt.oz[i]  = r.origin.z;
                pkt.dx[i]  = r.direction.x;  pkt.dy[i]  = r.direction.y;  pkt.dz[i]  = r.direction.z;
                pkt.idx[i] = 1.f / r.direction.x;
                pkt.idy[i] = 1.f / r.direction.y;
                pkt.idz[i] = 1.f / r.direction.z;
            }

            // Ergebnis-Arrays mit Sentinel-Werten initialisieren
            alignas(32) float best_t[8], best_u[8], best_v[8];
            int best_tri[8];
            for (int i = 0; i < 8; ++i) { best_t[i] = INF; best_u[i] = best_v[i] = 0.f; best_tri[i] = -1; }

            sc.kd.closest_hit_packet(sc.triangles, pkt, 1e-4f,
                                     best_t, best_u, best_v, best_tri);

            // Pixel shaden (skalarer Phong – Shadow Rays bleiben skalarer k-d-Baum)
            for (int i = 0; i < count; ++i) {
                Vec3 col;
                if (best_tri[i] >= 0) {
                    float s = (x + i + 0.5f) / W;
                    float t = (y + 0.5f) / H;
                    Ray r   = cam.get_ray(s, t);
                    col = shade(sc, r, best_tri[i], best_t[i], best_u[i], best_v[i]);
                } else {
                    col = sc.background;
                }
                img.set(x + i, H - 1 - y, col);
            }
        }

        if ((y + 1) % 50 == 0 || y + 1 == H)
            std::cout << "\rRendere (Packet)... " << (y + 1) << "/" << H << std::flush;
    }
    std::cout << "\n";
}
#endif // HAVE_AVX2

bool Image::save_ppm(const std::string& path) const {
    std::ofstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "Fehler: Kann nicht schreiben: " << path << "\n";
        return false;
    }
    f << "P6\n" << width << " " << height << "\n255\n";
    f.write(reinterpret_cast<const char*>(pixels.data()),
            static_cast<std::streamsize>(pixels.size()));
    return true;
}
