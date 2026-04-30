cmake_minimum_required(VERSION 3.10)
project(RaytracerLab CXX)

# 1. C++ Standard setzen (C++17 oder höher wird für moderne Features empfohlen)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Compiler-Optionen für GCC (g++)
# -Wall -Wextra: Schaltet Warnungen an (hilft bei der Fehlersuche)
# -O3: Maximale Optimierung für die Zeitmessung
# -mavx2: Aktiviert AVX-Befehlssätze (wichtig für Aufgabe 3!)
if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra)
    
    # Release-Modus optimieren
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mavx2")
    
    # Debug-Modus für die Entwicklung (keine Optimierung, aber Debug-Symbole)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

# 3. Bibliotheken finden oder einbinden
# Falls GLM im System installiert ist:
find_package(glm QUIET)

# Falls GLM nicht im System ist, nutze den Pfad im Projekt (Header-only)
if(NOT glm_FOUND)
    message(STATUS "GLM nicht im System gefunden, nutze lokalen Pfad in external/")
    include_directories(external)
endif()

# 4. Header-Verzeichnisse bekannt machen
include_directories(include)

# 5. Quelldateien sammeln
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# 6. Das ausführbare Programm erstellen
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 7. GLM verlinken (da header-only, reicht oft include_directories, aber sauberer so:)
if(glm_FOUND)
    target_link_libraries(${PROJECT_NAME} glm::glm)
endif()

# Zusätzliche Info für den Build-Prozess
message(STATUS "Build-Typ: ${CMAKE_BUILD_TYPE}")