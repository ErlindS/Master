#!/bin/bash
# =============================================================================
# Build- und Ausführungsskript für den Raytracer
# Verwendung: ./build_and_run.sh [debug|release|all]
# Standardmäßig wird "release" ausgeführt
# =============================================================================

set -e  # Skript bei Fehler abbrechen

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR_DEBUG="${PROJECT_DIR}/build-debug"
BUILD_DIR_RELEASE="${PROJECT_DIR}/build-release"
BUILD_DIR_DEFAULT="${PROJECT_DIR}/build"

# Farben für die Ausgabe
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

print_header() {
    echo ""
    echo -e "${CYAN}=============================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}=============================================${NC}"
    echo ""
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}→ $1${NC}"
}

# --- Funktion: Debug-Build ---
build_debug() {
    print_header "BUILD: Debug-Modus (ohne Optimierung, mit Debug-Symbolen)"

    mkdir -p "${BUILD_DIR_DEBUG}"
    cd "${BUILD_DIR_DEBUG}"

    print_info "CMake konfigurieren (Debug)..."
    cmake "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE=Debug 2>&1 | tail -3

    print_info "Kompilieren..."
    make -j$(nproc) 2>&1

    print_success "Debug-Build erfolgreich!"
    cd "${PROJECT_DIR}"
}

# --- Funktion: Release-Build ---
build_release() {
    print_header "BUILD: Release-Modus (mit -O3 Optimierung)"

    mkdir -p "${BUILD_DIR_RELEASE}"
    cd "${BUILD_DIR_RELEASE}"

    print_info "CMake konfigurieren (Release)..."
    cmake "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE=Release 2>&1 | tail -3

    print_info "Kompilieren..."
    make -j$(nproc) 2>&1

    print_success "Release-Build erfolgreich!"
    cd "${PROJECT_DIR}"
}

# --- Funktion: Standard-Build (bestehender build/-Ordner) ---
build_default() {
    print_header "BUILD: Standard (bestehender build/-Ordner)"

    mkdir -p "${BUILD_DIR_DEFAULT}"
    cd "${BUILD_DIR_DEFAULT}"

    print_info "CMake konfigurieren..."
    cmake "${PROJECT_DIR}" 2>&1 | tail -3

    print_info "Kompilieren..."
    make -j$(nproc) 2>&1

    print_success "Standard-Build erfolgreich!"
    cd "${PROJECT_DIR}"
}

# --- Funktion: Tests kompilieren und ausführen ---
run_tests() {
    print_header "TESTS: Kompilieren und Ausführen"

    mkdir -p "${BUILD_DIR_DEBUG}"
    cd "${BUILD_DIR_DEBUG}"

    print_info "CMake konfigurieren (Debug für Tests)..."
    cmake "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE=Debug 2>&1 | tail -3

    print_info "Kompiliere Tests (RaytracerTests)..."
    make RaytracerTests -j$(nproc) 2>&1

    if [ ! -f "RaytracerTests" ]; then
        echo -e "${RED}✗ Test-Executable (RaytracerTests) nicht gefunden in ${BUILD_DIR_DEBUG}${NC}"
        return 1
    fi

    print_info "Führe Tests aus..."
    echo ""
    ./RaytracerTests
    echo ""
    
    print_success "Tests abgeschlossen."
    cd "${PROJECT_DIR}"
}

# --- Funktion: Ausführung ---
run_raytracer() {
    local build_dir="$1"
    local mode="$2"

    print_header "AUSFÜHRUNG: ${mode}-Modus"

    if [ ! -f "${build_dir}/RaytracerLab" ]; then
        echo -e "${RED}✗ Executable nicht gefunden in ${build_dir}${NC}"
        return 1
    fi

    cd "${build_dir}"
    print_info "Starte Raytracer (${mode})..."
    echo ""

    # Ausführung mit time-Befehl für zusätzliche Systemmessung
    time ./RaytracerLab

    echo ""
    print_success "${mode}-Modus abgeschlossen."

    # PPM-Ausgabe prüfen und zu PNG konvertieren
    if [ -f "output.ppm" ]; then
        local filesize=$(du -h output.ppm | cut -f1)
        print_info "Ausgabedatei: output.ppm (${filesize})"
        
        # Konvertierung zu PNG
        if command -v magick &> /dev/null; then
            print_info "Konvertiere output.ppm zu output.png (mit magick)..."
            magick output.ppm output.png
            print_success "Erfolgreich konvertiert: output.png"
        elif command -v convert &> /dev/null; then
            print_info "Konvertiere output.ppm zu output.png (mit convert)..."
            convert output.ppm output.png
            print_success "Erfolgreich konvertiert: output.png"
        elif command -v ffmpeg &> /dev/null; then
            print_info "Konvertiere output.ppm zu output.png (mit ffmpeg)..."
            ffmpeg -y -v quiet -i output.ppm output.png
            print_success "Erfolgreich konvertiert: output.png"
        elif command -v pnmtopng &> /dev/null; then
            print_info "Konvertiere output.ppm zu output.png (mit pnmtopng)..."
            pnmtopng output.ppm > output.png
            print_success "Erfolgreich konvertiert: output.png"
        else
            print_info "Weder ImageMagick, ffmpeg noch netpbm gefunden. Überspringe die Konvertierung zu PNG."
        fi
    fi

    cd "${PROJECT_DIR}"
}

# --- Funktion: Ergebnistabelle ---
print_results() {
    print_header "ZUSAMMENFASSUNG DER ZEITMESSUNGEN"
    echo "Bitte die Renderzeiten aus den obigen Ausgaben entnehmen"
    echo "und in den Projektbericht eintragen."
    echo ""
    echo "Empfohlenes Format für den Bericht:"
    echo ""
    echo "  | Konfiguration              | Renderzeit [ms] |"
    echo "  |:---------------------------|----------------:|"
    echo "  | Debug   (ohne Optimierung) |          ????? |"
    echo "  | Release (mit -O3)          |          ????? |"
    echo ""
}

# --- Hauptprogramm ---
MODE="${1:-release}"

case "${MODE}" in
    debug)
        build_debug
        run_raytracer "${BUILD_DIR_DEBUG}" "Debug"
        ;;
    release)
        build_release
        run_raytracer "${BUILD_DIR_RELEASE}" "Release"
        ;;
    default)
        build_default
        run_raytracer "${BUILD_DIR_DEFAULT}" "Standard"
        ;;
    test)
        run_tests
        ;;
    all)
        build_debug
        run_raytracer "${BUILD_DIR_DEBUG}" "Debug"

        build_release
        run_raytracer "${BUILD_DIR_RELEASE}" "Release"

        print_results
        ;;
    clean)
        print_header "AUFRÄUMEN"
        rm -rf "${BUILD_DIR_DEBUG}" "${BUILD_DIR_RELEASE}"
        print_success "Build-Verzeichnisse entfernt."
        ;;
    *)
        echo "Verwendung: $0 [debug|release|default|test|all|clean]"
        echo ""
        echo "  debug   - Kompiliert und startet im Debug-Modus (keine Optimierung)"
        echo "  release - Kompiliert und startet im Release-Modus (-O3) (Standard)"
        echo "  default - Kompiliert und startet mit bestehendem build/-Ordner"
        echo "  test    - Kompiliert und führt die Google Tests aus (RaytracerTests)"
        echo "  all     - Führt Debug und Release nacheinander aus"
        echo "  clean   - Entfernt Debug- und Release-Build-Ordner"
        exit 1
        ;;
esac

echo ""
print_success "Fertig!"
