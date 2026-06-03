#include "obj_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

// Liest eine MTL-Datei und füllt Materialien in scene.materials.
static void load_mtl(const std::string& dir, const std::string& mtlfile, Scene& scene)
{
    std::string path = dir + mtlfile;
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "MTL-Datei nicht gefunden: " << path << "\n";
        return;
    }

    Material* cur = nullptr;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string kw;
        ss >> kw;

        if (kw == "newmtl") {
            std::string name;
            ss >> name;
            bool found = false;
            for (auto& m : scene.materials) {
                if (m.name == name) { cur = &m; found = true; break; }
            }
            if (!found) {
                scene.materials.push_back(Material{});
                scene.materials.back().name = name;
                cur = &scene.materials.back();
            }
        } else if (cur) {
            float x, y, z;
            if (kw == "Ka" && (ss >> x >> y >> z))  cur->ambient   = {x, y, z};
            if (kw == "Kd" && (ss >> x >> y >> z))  cur->diffuse   = {x, y, z};
            if (kw == "Ks" && (ss >> x >> y >> z))  cur->specular  = {x, y, z};
            if (kw == "Ns" && (ss >> x))             cur->shininess = x;
        }
    }
}

// Parst einen Face-Token wie "3", "3/2", "3//5" oder "3/2/5".
// Gibt Vertex-Index (1-basiert) und Normal-Index (0 wenn nicht angegeben) zurück.
static void parse_token(const std::string& tok, int& vi, int& ni) {
    vi = ni = 0;
    size_t p1 = tok.find('/');
    if (p1 == std::string::npos) {
        vi = std::stoi(tok);
        return;
    }
    vi = std::stoi(tok.substr(0, p1));

    size_t p2 = tok.find('/', p1 + 1);
    if (p2 == std::string::npos) return; // Format: v/vt — kein Normal

    std::string ns = tok.substr(p2 + 1);
    if (!ns.empty()) ni = std::stoi(ns);
}

// Wandelt 1-basierten OBJ-Index in 0-basierten C-Index um.
// Negative OBJ-Indizes sind relativ zum aktuellen Ende.
static int resolve(int idx, int size) {
    return (idx > 0) ? idx - 1 : size + idx;
}

bool load_obj(const std::string& path, Scene& scene) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "OBJ-Datei nicht gefunden: " << path << "\n";
        return false;
    }

    // Verzeichnis der OBJ-Datei für relative MTL-Pfade ermitteln
    std::string dir;
    size_t slash = path.find_last_of("/\\");
    if (slash != std::string::npos) dir = path.substr(0, slash + 1);

    std::vector<Vec3> verts;
    std::vector<Vec3> norms;
    int cur_mat = 0;

    // Sicherstellen, dass mindestens ein Defaultmaterial existiert
    if (scene.materials.empty())
        scene.materials.push_back(Material{});

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string kw;
        ss >> kw;

        if (kw == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            verts.push_back({x, y, z});

        } else if (kw == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            norms.push_back(Vec3{x, y, z}.normalized());

        } else if (kw == "usemtl") {
            // Neues Material – wir fügen ein leeres ein wenn nötig
            std::string mname;
            ss >> mname;
            bool found = false;
            for (int i = 0; i < (int)scene.materials.size(); ++i) {
                if (scene.materials[i].name == mname) { cur_mat = i; found = true; break; }
            }
            if (!found) {
                Material m;
                m.name = mname;
                cur_mat = (int)scene.materials.size();
                scene.materials.push_back(m);
            }

        } else if (kw == "f") {
            // Face: beliebig viele Vertices, Fan-Triangulierung
            std::vector<int> vis, nis;
            std::string tok;
            while (ss >> tok) {
                int vi, ni;
                parse_token(tok, vi, ni);
                vis.push_back(resolve(vi, (int)verts.size()));
                nis.push_back(ni != 0 ? resolve(ni, (int)norms.size()) : -1);
            }

            if ((int)vis.size() < 3) continue;

            for (int i = 1; i + 1 < (int)vis.size(); ++i) {
                // Bounds-Check
                if (vis[0] < 0 || vis[0] >= (int)verts.size()) continue;
                if (vis[i] < 0 || vis[i] >= (int)verts.size()) continue;
                if (vis[i+1] < 0 || vis[i+1] >= (int)verts.size()) continue;

                Triangle tri;
                tri.v0     = verts[vis[0]];
                tri.v1     = verts[vis[i]];
                tri.v2     = verts[vis[i+1]];
                tri.mat_id = cur_mat;

                // Flächennormale als Fallback (wenn keine Vertex-Normalen)
                Vec3 fn = (tri.v1 - tri.v0).cross(tri.v2 - tri.v0).normalized();

                auto get_n = [&](int k) -> Vec3 {
                    int ni = nis[k];
                    return (ni >= 0 && ni < (int)norms.size()) ? norms[ni] : fn;
                };
                tri.n0 = get_n(0);
                tri.n1 = get_n(i);
                tri.n2 = get_n(i + 1);

                scene.triangles.push_back(tri);
            }
        } else if (kw == "mtllib") {
            std::string mtlname;
            ss >> mtlname;
            load_mtl(dir, mtlname, scene);
        }
        // g, s, o: ignorieren
    }

    std::cout << "OBJ geladen: " << verts.size() << " Vertices, "
              << scene.triangles.size() << " Dreiecke\n";
    return !scene.triangles.empty();
}
