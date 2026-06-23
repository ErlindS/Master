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

glm::vec3 Renderer::traceRay(const Ray& ray, const Mesh& scene, const Light& light) {
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

        // Ambient + Diffuse + Specular zusammenrechnen
        glm::vec3 finalColor = glm::clamp(ambientColor + finalDiffuse + finalSpecular, 0.0f, 1.0f);
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
    const int N_AA_SAMPLES = 4;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.5f, 0.5f);
    
    constexpr int packetSize = floatv::size();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += packetSize) {
            glm::vec3 finalColors[packetSize] = {glm::vec3(0.0f)};
            
            for (int s = 0; s < N_AA_SAMPLES; ++s) {
                float ox[packetSize], oy[packetSize], oz[packetSize];
                float dx[packetSize], dy[packetSize], dz[packetSize];
                float invDx[packetSize], invDy[packetSize], invDz[packetSize];
                bool active[packetSize];

                // Paket befüllen
                for(int i = 0; i < packetSize; ++i) {
                    int px = x + i;
                    if (px < width) {
                        float jitterX = dis(gen);
                        float jitterY = dis(gen);
                        float u = (float)(px + jitterX) / (width - 1);
                        float v = (float)(y + jitterY) / (height - 1);

                        Ray ray = cam.generateRay(u, v);
                        ox[i] = ray.origin.x; oy[i] = ray.origin.y; oz[i] = ray.origin.z;
                        dx[i] = ray.direction.x; dy[i] = ray.direction.y; dz[i] = ray.direction.z;
                        invDx[i] = ray.invDirection.x; invDy[i] = ray.invDirection.y; invDz[i] = ray.invDirection.z;
                        active[i] = true;
                    } else {
                        ox[i] = 0; oy[i] = 0; oz[i] = 0;
                        dx[i] = 1; dy[i] = 0; dz[i] = 0;
                        invDx[i] = 1; invDy[i] = 0; invDz[i] = 0;
                        active[i] = false;
                    }
                }

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
                
                maskv activeMask(false);
                for(int i=0; i<packetSize; ++i) if(active[i]) activeMask[i] = true;

                IntersectionPacket closestIsect;

                if (useAcceleration && scene.kdtree) {
                    scene.kdtree->intersectPacket(rayPacket, activeMask, closestIsect, algorithm);
                } else {
                    // Ohne Beschleunigungsstruktur wird SIMD ignoriert
                    for(int i=0; i<packetSize; ++i) {
                        if (active[i]) {
                            Ray r(glm::vec3(ox[i], oy[i], oz[i]), glm::vec3(dx[i], dy[i], dz[i]));
                            Intersection isect; Triangle tri;
                            if (findClosestHit(r, scene, isect, tri)) {
                                closestIsect.hit[i] = true;
                                closestIsect.t[i] = isect.t;
                                closestIsect.u[i] = isect.u;
                                closestIsect.v[i] = isect.v;
                                // TriIndex suchen (ineffizient aber nur Fallback)
                                for(size_t tIdx=0; tIdx<scene.triangles.size(); ++tIdx) {
                                    if(scene.triangles[tIdx].v0 == tri.v0 && scene.triangles[tIdx].v1 == tri.v1) {
                                        closestIsect.triIndex[i] = (float)tIdx; break;
                                    }
                                }
                            }
                        }
                    }
                }

                maskv validHit = activeMask && closestIsect.hit;

                if (stdx::any_of(validHit)) {
                    // Gather-Phase: Daten für die getroffenen Dreiecke extrahieren
                    float hx[packetSize], hy[packetSize], hz[packetSize];
                    float nx[packetSize], ny[packetSize], nz[packetSize];
                    float cr[packetSize], cg[packetSize], cb[packetSize];
                    float sr[packetSize], sg[packetSize], sb[packetSize];
                    float se[packetSize];

                    for(int i=0; i<packetSize; ++i) {
                        if (validHit[i]) {
                            int triIdx = (int)closestIsect.triIndex[i];
                            Triangle tri = scene.triangles[triIdx];
                            
                            float t = closestIsect.t[i];
                            float hitX = ox[i] + dx[i] * t;
                            float hitY = oy[i] + dy[i] * t;
                            float hitZ = oz[i] + dz[i] * t;
                            hx[i] = hitX; hy[i] = hitY; hz[i] = hitZ;

                            float u = closestIsect.u[i];
                            float v = closestIsect.v[i];
                            float w = 1.0f - u - v;
                            glm::vec3 n = glm::normalize(w * tri.n0 + u * tri.n1 + v * tri.n2);
                            
                            // View direction
                            glm::vec3 viewDir = glm::normalize(glm::vec3(ox[i], oy[i], oz[i]) - glm::vec3(hitX, hitY, hitZ));
                            if (glm::dot(n, viewDir) < 0.0f) n = -n;

                            nx[i] = n.x; ny[i] = n.y; nz[i] = n.z;
                            cr[i] = tri.color.r; cg[i] = tri.color.g; cb[i] = tri.color.b;
                            sr[i] = tri.specularColor.r; sg[i] = tri.specularColor.g; sb[i] = tri.specularColor.b;
                            se[i] = tri.specularExponent;
                        } else {
                            hx[i] = 0; hy[i] = 0; hz[i] = 0;
                            nx[i] = 0; ny[i] = 0; nz[i] = 0;
                            cr[i] = 0; cg[i] = 0; cb[i] = 0;
                            sr[i] = 0; sg[i] = 0; sb[i] = 0;
                            se[i] = 0;
                        }
                    }

                    floatv hitX(hx, stdx::element_aligned);
                    floatv hitY(hy, stdx::element_aligned);
                    floatv hitZ(hz, stdx::element_aligned);
                    
                    floatv normX(nx, stdx::element_aligned);
                    floatv normY(ny, stdx::element_aligned);
                    floatv normZ(nz, stdx::element_aligned);

                    // Lichtrichtung berechnen (SIMD)
                    floatv lx = light.position.x - hitX;
                    floatv ly = light.position.y - hitY;
                    floatv lz = light.position.z - hitZ;
                    floatv distToLight = stdx::sqrt(lx*lx + ly*ly + lz*lz);
                    lx /= distToLight;
                    ly /= distToLight;
                    lz /= distToLight;

                    // Schattenstrahlen-Paket generieren
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

                    IntersectionPacket shadowIsect;
                    if (useAcceleration && scene.kdtree) {
                        scene.kdtree->intersectPacket(shadowPacket, validHit, shadowIsect, algorithm);
                    }

                    // Maske für Pixel im Schatten
                    maskv inShadow = shadowIsect.hit && (shadowIsect.t < distToLight);
                    maskv lit = validHit && !inShadow;

                    floatv diff = stdx::max(floatv(0.0f), normX * lx + normY * ly + normZ * lz);
                    
                    floatv colorR(cr, stdx::element_aligned);
                    floatv colorG(cg, stdx::element_aligned);
                    floatv colorB(cb, stdx::element_aligned);
                    
                    // Ambiente Beleuchtung
                    floatv finalR = colorR * 0.15f;
                    floatv finalG = colorG * 0.15f;
                    floatv finalB = colorB * 0.15f;

                    // Diffuse Beleuchtung
                    maskv applyDiff = lit && (diff > 0.0f);
                    stdx::where(applyDiff, finalR) += colorR * diff;
                    stdx::where(applyDiff, finalG) += colorG * diff;
                    stdx::where(applyDiff, finalB) += colorB * diff;

                    // Spekulare Beleuchtung
                    floatv specExp(se, stdx::element_aligned);
                    maskv applySpec = applyDiff && (specExp > 0.0f);
                    if (stdx::any_of(applySpec)) {
                        floatv vx = rayPacket.ox - hitX;
                        floatv vy = rayPacket.oy - hitY;
                        floatv vz = rayPacket.oz - hitZ;
                        floatv vLen = stdx::sqrt(vx*vx + vy*vy + vz*vz);
                        vx /= vLen; vy /= vLen; vz /= vLen;

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

                    // Clamp
                    finalR = stdx::clamp(finalR, floatv(0.0f), floatv(1.0f)) * 255.0f;
                    finalG = stdx::clamp(finalG, floatv(0.0f), floatv(1.0f)) * 255.0f;
                    finalB = stdx::clamp(finalB, floatv(0.0f), floatv(1.0f)) * 255.0f;

                    // Zurück in Farben-Array schreiben
                    for(int i=0; i<packetSize; ++i) {
                        if (active[i]) {
                            if (validHit[i]) {
                                finalColors[i] += glm::vec3(finalR[i], finalG[i], finalB[i]);
                            } else {
                                finalColors[i] += glm::vec3(0.05f, 0.05f, 0.15f) * 255.0f;
                            }
                        }
                    }
                } else {
                    // Kein einziger Treffer im Paket
                    for(int i=0; i<packetSize; ++i) {
                        if (active[i]) {
                            finalColors[i] += glm::vec3(0.05f, 0.05f, 0.15f) * 255.0f;
                        }
                    }
                }
            }
            
            for(int i = 0; i < packetSize; ++i) {
                if(x + i < width) {
                    finalColors[i] /= (float)N_AA_SAMPLES;
                    image.setPixel(x + i, y, finalColors[i]);
                }
            }
        }
    }
}
