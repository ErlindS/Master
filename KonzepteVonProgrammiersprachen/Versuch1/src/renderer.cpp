#include "renderer.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <random>

#include "kdtree.h"
#include "packet.h"

bool Renderer::findClosestHit(const Ray& ray, const Mesh& scene, Intersection& closestIsect, Triangle& hitTriangle) {
    if (useAcceleration && scene.kdtree) {
        return scene.kdtree->intersect(ray, closestIsect, hitTriangle, algorithm);
    }
    
    // Fallback: Lineare Suche, falls kein KD-Baum vorhanden oder deaktiviert ist
    closestIsect.hit = false;
    closestIsect.t = std::numeric_limits<float>::max();
    bool hitAnything = false;

    for (const auto& tri : scene.triangles) {
        Intersection isect;
        if (algorithm == IntersectionAlgorithm::BADOUEL) {
            isect = Intersector::intersectRayTriangleBadouel(ray, tri.v0, tri.v1, tri.v2);
        } else {
            isect = Intersector::intersectRayTriangle(ray, tri.v0, tri.v1, tri.v2);
        }
        
        if (isect.hit && isect.t < closestIsect.t) {
            closestIsect = isect;
            hitTriangle = tri;
            hitAnything = true;
        }
    }
    return hitAnything;
}

glm::vec3 Renderer::traceRay(const Ray& ray, const Mesh& scene, const Light& light, int depth) {
    Intersection isect;
    Triangle hitTriangle;

    if (findClosestHit(ray, scene, isect, hitTriangle)) {
        // Exakter Punkt im 3D-Raum (Ursprung + Richtung * Distanz)
        glm::vec3 hitPoint = ray.origin + ray.direction * isect.t;

        // Oberflächennormale berechnen (Interpoliert mit baryzentrischen Koordinaten)
        float w = 1.0f - isect.u - isect.v;
        glm::vec3 normal = glm::normalize(w * hitTriangle.n0 + isect.u * hitTriangle.n1 + isect.v * hitTriangle.n2);

        // Blickrichtung berechnen
        glm::vec3 viewDir = glm::normalize(ray.origin - hitPoint);

        // Normale zur Kamera hin drehen (Double-Sided Shading)
        if (glm::dot(normal, viewDir) < 0.0f) {
            normal = -normal;
        }
        const int N_SHADOW_SAMPLES = 8;
        const glm::vec3 lightPos = light.position;
        glm::vec3 diffuseSum(0.0f);
        glm::vec3 specularSum(0.0f);

        for (int i = 0; i < N_SHADOW_SAMPLES; ++i) {
            // Exakte Lichtposition (Punktlichtquelle)
            glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);
            float distanceToLight = glm::length(lightPos - hitPoint);
            
            // Schattenstrahl (Shadow Ray)
            Ray shadowRay(hitPoint + normal * 0.001f, lightDir);
            Intersection shadowIsect;
            Triangle dummyTriangle;
            bool inShadow = findClosestHit(shadowRay, scene, shadowIsect, dummyTriangle);

            if (inShadow && shadowIsect.t > distanceToLight) {
                inShadow = false;
            }

            if (!inShadow) {
                // Diffuse (Lambert)
                float diff = std::max(glm::dot(normal, lightDir), 0.0f);
                diffuseSum += hitTriangle.color * diff;

                // Specular (Blinn-Phong)
                if (hitTriangle.specularExponent > 0.0f && diff > 0.0f) {
                    glm::vec3 halfDir = glm::normalize(lightDir + viewDir);
                    float specAngle = std::max(glm::dot(normal, halfDir), 0.0f);
                    float specFactor = std::pow(specAngle, hitTriangle.specularExponent);
                    specularSum += hitTriangle.specularColor * specFactor;
                }
            }
        }

        // 15% Umgebungslicht
        glm::vec3 ambientColor = hitTriangle.color * 0.15f;
        
        // Durchschnittliche Beleuchtung über alle Schatten-Samples berechnen
        glm::vec3 finalDiffuse = diffuseSum / (float)N_SHADOW_SAMPLES;
        glm::vec3 finalSpecular = specularSum / (float)N_SHADOW_SAMPLES;

        // Lokale Farbe (Ambient + Diffuse + Specular)
        glm::vec3 localColor = ambientColor + finalDiffuse + finalSpecular;

        // Spiegelung (spekulare Reflexion) rekursiv hinzufügen
        glm::vec3 reflectedColor(0.0f);
        if (depth < 4 && glm::length(hitTriangle.specularColor) > 0.0f) {
            glm::vec3 reflectDir = glm::reflect(ray.direction, normal);
            Ray reflectRay(hitPoint + normal * 0.001f, reflectDir);
            reflectedColor = traceRay(reflectRay, scene, light, depth + 1) / 255.0f;
        }

        // Lokale Farbe mit reflektierter Farbe mischen
        glm::vec3 finalColor = glm::clamp(localColor + hitTriangle.specularColor * reflectedColor, 0.0f, 1.0f);
        return finalColor * 255.0f;
    }
    
    // Hintergrundfarbe (Dunkelblau wie im Referenzbild)
    return glm::vec3(0.05f, 0.05f, 0.15f) * 255.0f;
}

void Renderer::render(const Mesh& scene, const Camera& cam, const Light& light, Image& image) {
    int width = image.getWidth();
    int height = image.getHeight();
    const int N_AA_SAMPLES = 4;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.5f, 0.5f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            glm::vec3 finalColor(0.0f);
            
            // Anti-Aliasing Loop (Multisampling)
            for (int s = 0; s < N_AA_SAMPLES; ++s) {
                float jitterX = dis(gen);
                float jitterY = dis(gen);
                float u = (float)(x + jitterX) / (width - 1);
                float v = (float)(y + jitterY) / (height - 1);

                Ray ray = cam.generateRay(u, v);
                finalColor += traceRay(ray, scene, light);
            }
            
            // Durchschnittliche Farbe der Samples
            finalColor /= (float)N_AA_SAMPLES;
            
            image.setPixel(x, y, finalColor);
        }
    }
}

void Renderer::renderSIMD(const Mesh& scene, const Camera& cam, const Light& light, Image& image) {
    int width = image.getWidth();
    int height = image.getHeight();
    const int N_AA_SAMPLES = 4; // 4x Multisampling (Jittering) für Antialiasing

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.5f, 0.5f);
    
    constexpr int packetSize = floatv::size(); // Dynamisch ermittelte Vektorbreite (z.B. 8 bei AVX2)

    // 1. Äußere Schleifen über alle Pixel des Bildes
    for (int y = 0; y < height; ++y) {
        // Wir springen in Schritten von packetSize, da wir ein Pixelpaket parallel verarbeiten
        for (int x = 0; x < width; x += packetSize) {
            // Farben-Puffer für die Pixel im aktuellen Paket initialisieren
            glm::vec3 finalColors[packetSize] = {glm::vec3(0.0f)};
            
            // 2. Schleife für das Anti-Aliasing (Multisampling)
            for (int s = 0; s < N_AA_SAMPLES; ++s) {
                float ox[packetSize], oy[packetSize], oz[packetSize];
                float dx[packetSize], dy[packetSize], dz[packetSize];
                float invDx[packetSize], invDy[packetSize], invDz[packetSize];
                bool active[packetSize];

                // 3. Paket befüllen (Laden der skalaren Strahlendaten in Arrays)
                for(int i = 0; i < packetSize; ++i) {
                    int px = x + i;
                    if (px < width) {
                        // Pixelkoordinaten leicht verschieben (Jitter) für Antialiasing
                        float jitterX = dis(gen);
                        float jitterY = dis(gen);
                        float u = (float)(px + jitterX) / (width - 1);
                        float v = (float)(y + jitterY) / (height - 1);

                        // Skalaren Strahl über die Kamera generieren
                        Ray ray = cam.generateRay(u, v);
                        ox[i] = ray.origin.x; oy[i] = ray.origin.y; oz[i] = ray.origin.z;
                        dx[i] = ray.direction.x; dy[i] = ray.direction.y; dz[i] = ray.direction.z;
                        invDx[i] = ray.invDirection.x; invDy[i] = ray.invDirection.y; invDz[i] = ray.invDirection.z;
                        active[i] = true;
                    } else {
                        // Padding-Strahlen für den Fall, dass die Bildbreite kein Vielfaches von packetSize ist
                        ox[i] = 0; oy[i] = 0; oz[i] = 0;
                        dx[i] = 1; dy[i] = 0; dz[i] = 0;
                        invDx[i] = 1; invDy[i] = 0; invDz[i] = 0;
                        active[i] = false;
                    }
                }

                // Skalare Arrays in SIMD-Vektortypen konvertieren
                RayPacket rayPacket;
                rayPacket.ox = floatv(ox, stdx::element_aligned);
                rayPacket.oy = floatv(oy, stdx::element_aligned);
                rayPacket.oz = floatv(oz, stdx::element_aligned);
                rayPacket.dx = floatv(dx, stdx::element_aligned);
                rayPacket.dy = floatv(dy, stdx::element_aligned);
                rayPacket.dz = floatv(dz, stdx::element_aligned);
                rayPacket.invDx = floatv(invDx, stdx::element_aligned);
                rayPacket.invDy = floatv(invDy, stdx::element_aligned);
                rayPacket.invDz = floatv(invDz, stdx::element_aligned);
                
                // Aktive Strahlenmaske erstellen
                maskv activeMask(false);
                for(int i=0; i<packetSize; ++i) if(active[i]) activeMask[i] = true;

                IntersectionPacket closestIsect;

                // 4. Schnittpunktsuche im k-d-Baum mit dem gesamten Strahlpaket
                if (useAcceleration && scene.kdtree) {
                    scene.kdtree->intersectPacket(rayPacket, activeMask, closestIsect, algorithm);
                } else {
                    // Fallback: Skalare Schnittpunktsuche, falls die Beschleunigungsstruktur deaktiviert ist
                    for(int i=0; i<packetSize; ++i) {
                        if (active[i]) {
                            Ray r(glm::vec3(ox[i], oy[i], oz[i]), glm::vec3(dx[i], dy[i], dz[i]));
                            Intersection isect; Triangle tri;
                            if (findClosestHit(r, scene, isect, tri)) {
                                closestIsect.hit[i] = true;
                                closestIsect.t[i] = isect.t;
                                closestIsect.u[i] = isect.u;
                                closestIsect.v[i] = isect.v;
                                for(size_t tIdx=0; tIdx<scene.triangles.size(); ++tIdx) {
                                    if(scene.triangles[tIdx].v0 == tri.v0 && scene.triangles[tIdx].v1 == tri.v1) {
                                        closestIsect.triIndex[i] = (float)tIdx; break;
                                    }
                                }
                            }
                        }
                    }
                }

                // Maske für Strahlen, die tatsächlich Geometrie getroffen haben
                maskv validHit = activeMask && closestIsect.hit;

                // 5. Shading und Beleuchtung (wird nur ausgeführt, wenn mindestens ein Strahl getroffen hat)
                if (stdx::any_of(validHit)) {
                    // Temporäre Arrays für die Gather-Phase
                    float hx[packetSize], hy[packetSize], hz[packetSize];
                    float nx[packetSize], ny[packetSize], nz[packetSize];
                    float cr[packetSize], cg[packetSize], cb[packetSize];
                    float sr[packetSize], sg[packetSize], sb[packetSize];
                    float se[packetSize];

                    // Gather-Phase: Auslesen der Dreiecks- und Schattierungsdaten für getroffene Pixel
                    for(int i=0; i<packetSize; ++i) {
                        if (validHit[i]) {
                            int triIdx = (int)closestIsect.triIndex[i];
                            Triangle tri = scene.triangles[triIdx];
                            
                            // Treffpunkt im 3D-Raum berechnen
                            float t = closestIsect.t[i];
                            float hitX = ox[i] + dx[i] * t;
                            float hitY = oy[i] + dy[i] * t;
                            float hitZ = oz[i] + dz[i] * t;
                            hx[i] = hitX; hy[i] = hitY; hz[i] = hitZ;

                            // Normaleninterpolation mittels baryzentrischen Koordinaten
                            float u = closestIsect.u[i];
                            float v = closestIsect.v[i];
                            float w = 1.0f - u - v;
                            glm::vec3 n = glm::normalize(w * tri.n0 + u * tri.n1 + v * tri.n2);
                            
                            // View-Richtung berechnen und Normale bei Bedarf umdrehen (Double-Sided Shading)
                            glm::vec3 viewDir = glm::normalize(glm::vec3(ox[i], oy[i], oz[i]) - glm::vec3(hitX, hitY, hitZ));
                            if (glm::dot(n, viewDir) < 0.0f) n = -n;

                            nx[i] = n.x; ny[i] = n.y; nz[i] = n.z;
                            cr[i] = tri.color.r; cg[i] = tri.color.g; cb[i] = tri.color.b;
                            sr[i] = tri.specularColor.r; sg[i] = tri.specularColor.g; sb[i] = tri.specularColor.b;
                            se[i] = tri.specularExponent;
                        } else {
                            // Standardwerte für nicht-getroffene Strahlen
                            hx[i] = 0; hy[i] = 0; hz[i] = 0;
                            nx[i] = 0; ny[i] = 0; nz[i] = 0;
                            cr[i] = 0; cg[i] = 0; cb[i] = 0;
                            sr[i] = 0; sg[i] = 0; sb[i] = 0;
                            se[i] = 0;
                        }
                    }

                    // Laden der extrahierten Attribute in SIMD-Vektoren
                    floatv hitX(hx, stdx::element_aligned);
                    floatv hitY(hy, stdx::element_aligned);
                    floatv hitZ(hz, stdx::element_aligned);
                    
                    floatv normX(nx, stdx::element_aligned);
                    floatv normY(ny, stdx::element_aligned);
                    floatv normZ(nz, stdx::element_aligned);

                    // 6. Lichtrichtung & Schattenstrahlen (SIMD)
                    floatv lx = light.position.x - hitX;
                    floatv ly = light.position.y - hitY;
                    floatv lz = light.position.z - hitZ;
                    floatv distToLight = stdx::sqrt(lx*lx + ly*ly + lz*lz);
                    lx /= distToLight;
                    ly /= distToLight;
                    lz /= distToLight;

                    // Schattenstrahlen-Paket vorbereiten (leicht versetzt zur Vermeidung von Shadow Acne)
                    RayPacket shadowPacket;
                    shadowPacket.ox = hitX + normX * 0.001f;
                    shadowPacket.oy = hitY + normY * 0.001f;
                    shadowPacket.oz = hitZ + normZ * 0.001f;
                    shadowPacket.dx = lx;
                    shadowPacket.dy = ly;
                    shadowPacket.dz = lz;
                    shadowPacket.invDx = 1.0f / lx;
                    shadowPacket.invDy = 1.0f / ly;
                    shadowPacket.invDz = 1.0f / lz;

                    // Schattenstrahlpaket durch den k-d-Baum traversieren
                    IntersectionPacket shadowIsect;
                    if (useAcceleration && scene.kdtree) {
                        scene.kdtree->intersectPacket(shadowPacket, validHit, shadowIsect, algorithm);
                    }

                    // Maske für bebeleuchtete/unverschattete Strahlen
                    maskv inShadow = shadowIsect.hit && (shadowIsect.t < distToLight);
                    maskv lit = validHit && !inShadow;

                    // 7. Lokale Beleuchtung berechnen (Lambert-Diffuse & Blinn-Phong Specular)
                    floatv diff = stdx::max(floatv(0.0f), normX * lx + normY * ly + normZ * lz);
                    
                    floatv colorR(cr, stdx::element_aligned);
                    floatv colorG(cg, stdx::element_aligned);
                    floatv colorB(cb, stdx::element_aligned);
                    
                    // 15% ambienter Grundlichtanteil
                    floatv finalR = colorR * 0.15f;
                    floatv finalG = colorG * 0.15f;
                    floatv finalB = colorB * 0.15f;

                    // Diffusen Lichtanteil aufaddieren
                    maskv applyDiff = lit && (diff > 0.0f);
                    stdx::where(applyDiff, finalR) += colorR * diff;
                    stdx::where(applyDiff, finalG) += colorG * diff;
                    stdx::where(applyDiff, finalB) += colorB * diff;

                    // Spekularen Lichtanteil aufaddieren (falls Glanzeigenschaften vorhanden)
                    floatv specExp(se, stdx::element_aligned);
                    maskv applySpec = applyDiff && (specExp > 0.0f);
                    if (stdx::any_of(applySpec)) {
                        floatv vx = rayPacket.ox - hitX;
                        floatv vy = rayPacket.oy - hitY;
                        floatv vz = rayPacket.oz - hitZ;
                        floatv vLen = stdx::sqrt(vx*vx + vy*vy + vz*vz);
                        vx /= vLen; vy /= vLen; vz /= vLen;

                        // Half-Vektor berechnen (Blinn-Phong)
                        floatv hx_half = lx + vx;
                        floatv hy_half = ly + vy;
                        floatv hz_half = lz + vz;
                        floatv hLen = stdx::sqrt(hx_half*hx_half + hy_half*hy_half + hz_half*hz_half);
                        hx_half /= hLen; hy_half /= hLen; hz_half /= hLen;

                        floatv specAngle = stdx::max(floatv(0.0f), normX * hx_half + normY * hy_half + normZ * hz_half);
                        floatv specFactor = stdx::pow(specAngle, specExp);

                        floatv specR(sr, stdx::element_aligned);
                        floatv specG(sg, stdx::element_aligned);
                        floatv specB(sb, stdx::element_aligned);

                        stdx::where(applySpec, finalR) += specR * specFactor;
                        stdx::where(applySpec, finalG) += specG * specFactor;
                        stdx::where(applySpec, finalB) += specB * specFactor;
                    }

                    // 8. Spiegelungen (spekulare Reflexionen) rekursiv berechnen
                    float refR[packetSize] = {0.0f}, refG[packetSize] = {0.0f}, refB[packetSize] = {0.0f};
                    for(int i=0; i<packetSize; ++i) {
                        if (validHit[i]) {
                            int triIdx = (int)closestIsect.triIndex[i];
                            Triangle tri = scene.triangles[triIdx];
                            if (glm::length(tri.specularColor) > 0.0f) {
                                glm::vec3 normal(nx[i], ny[i], nz[i]);
                                glm::vec3 rayDir(dx[i], dy[i], dz[i]);
                                glm::vec3 hitPoint(hx[i], hy[i], hz[i]);
                                
                                // Reflektierten Strahlvektor berechnen und sequenziell weiterverfolgen
                                glm::vec3 reflectDir = glm::reflect(rayDir, normal);
                                Ray reflectRay(hitPoint + normal * 0.001f, reflectDir);
                                
                                // Rekursive Verfolgung startet hier auf Tiefe 1 (Skalarer Fallback)
                                glm::vec3 reflectColor = traceRay(reflectRay, scene, light, 1) / 255.0f;
                                glm::vec3 addedColor = tri.specularColor * reflectColor;
                                refR[i] = addedColor.r;
                                refG[i] = addedColor.g;
                                refB[i] = addedColor.b;
                            }
                        }
                    }
                    floatv reflectionR(refR, stdx::element_aligned);
                    floatv reflectionG(refG, stdx::element_aligned);
                    floatv reflectionB(refB, stdx::element_aligned);
                    
                    // Rekursive Spiegelungsfarbe auf die Farbkanäle aufaddieren
                    finalR += reflectionR;
                    finalG += reflectionG;
                    finalB += reflectionB;

                    // Farbwerte auf [0.0, 1.0] begrenzen und auf [0, 255] skalieren
                    finalR = stdx::clamp(finalR, floatv(0.0f), floatv(1.0f)) * 255.0f;
                    finalG = stdx::clamp(finalG, floatv(0.0f), floatv(1.0f)) * 255.0f;
                    finalB = stdx::clamp(finalB, floatv(0.0f), floatv(1.0f)) * 255.0f;

                    // 9. Ergebnisse in das Bild-Array zurückschreiben
                    for(int i=0; i<packetSize; ++i) {
                        if (active[i]) {
                            if (validHit[i]) {
                                finalColors[i] += glm::vec3(finalR[i], finalG[i], finalB[i]);
                            } else {
                                // Hintergrundfarbe setzen, falls der Strahl Geometrie verfehlt hat
                                finalColors[i] += glm::vec3(0.05f, 0.05f, 0.15f) * 255.0f;
                            }
                        }
                    }
                } else {
                    // Fall, dass das gesamte Paket ins Unendliche ging (kein einziger Treffer)
                    for(int i=0; i<packetSize; ++i) {
                        if (active[i]) {
                            finalColors[i] += glm::vec3(0.05f, 0.05f, 0.15f) * 255.0f;
                        }
                    }
                }
            }
            
            // Durchschnittsfarbe über alle AA-Proben berechnen und Pixel schreiben
            for(int i = 0; i < packetSize; ++i) {
                if(x + i < width) {
                    finalColors[i] /= (float)N_AA_SAMPLES;
                    image.setPixel(x + i, y, finalColors[i]);
                }
            }
        }
    }
}
