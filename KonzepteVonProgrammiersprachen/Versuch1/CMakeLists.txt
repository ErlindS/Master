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

# 5. Quelldateien sammeln (ohne main.cpp für die Tests)
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# 6. Das ausführbare Hauptprogramm erstellen
add_executable(${PROJECT_NAME} ${SOURCES} src/main.cpp ${HEADERS})

# 7. GLM verlinken (da header-only, reicht oft include_directories, aber sauberer so:)
if(glm_FOUND)
    target_link_libraries(${PROJECT_NAME} glm::glm)
endif()

# Zusätzliche Info für den Build-Prozess
message(STATUS "Build-Typ: ${CMAKE_BUILD_TYPE}")

# ==============================================================================
# TESTS (Google Test)
# ==============================================================================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

if(TEST_SOURCES)
    add_executable(RaytracerTests ${TEST_SOURCES} ${SOURCES} ${HEADERS})
    target_link_libraries(RaytracerTests gtest_main)
    if(glm_FOUND)
        target_link_libraries(RaytracerTests glm::glm)
    endif()
    
    include(GoogleTest)
    gtest_discover_tests(RaytracerTests)
endif()