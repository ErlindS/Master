# For debug (asserts are on)
#  rm CMakeCache.txt
#  cmake . 
# For release
#  rm CMakeCache.txt
#  cmake -DCMAKE_BUILD_TYPE=Release .  
#
# Do not forget to remove CMakeCache.txt after modifying the CMakeLists.txt
#
# rebuild everything
#    cmake --build .  --clean-first
cmake_minimum_required(VERSION 3.13)
project(Raytracing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Locate GoogleTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Source files for tests
file(GLOB TEST_SOURCES "test/*_test.cpp")

# Filter out the raytracer main files from GeometryLib
list(FILTER GEOMETRYLIB_SOURCES EXCLUDE REGEX "src/raytracer.*\\.cpp")

set(GEOMETRYLIB_SOURCES
	src/brute_force.cpp
	src/camera_inst.cpp
	src/geometry_inst.cpp
	src/kd_tree_inst.cpp
	src/matrix.cpp
	src/optical_geometry.cpp
	src/path_tracer.cpp
	src/raytracer.cpp
	src/shading_algorithms_inst.cpp
	src/vector.cpp
	src/camera.cpp
	src/geometry.cpp
	src/kd_tree.cpp
	src/logger.cpp
	src/obj_loader.cpp
	src/optical_geometry_inst.cpp
	src/ppm_reader.cpp
	src/shading_algorithms.cpp
	src/statistics.cpp
	src/vector_inst.cpp
)

# Add raytracer source files for executables
set(EXECUTABLE_SOURCES
	src/raytracer_main.cpp
	src/raytracer_path_tracer.cpp
	src/raytracer_triangle.cpp
	src/raytracer_triangle_normal.cpp
)

# Create the GeometryLib static library
add_library(GeometryLib STATIC ${GEOMETRYLIB_SOURCES})

# Create the executable targets
foreach(executable_source ${EXECUTABLE_SOURCES})
    get_filename_component(executable_name ${executable_source} NAME_WE)
    add_executable(${executable_name} ${executable_source})
    target_link_libraries(${executable_name} GeometryLib)
endforeach()

# Test executable: create, source, test and link
add_executable(Test ${TEST_SOURCES})

# Link against GeometryLib and GTest
target_link_libraries(Test GeometryLib gtest gtest_main)
enable_testing()
include(GoogleTest)
gtest_discover_tests(Test)

# Doxygen documentation configuration
find_package(Doxygen)
if(DOXYGEN_FOUND)
    # Set input and output files
    set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/test)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)

    # Doxygen configuration file
    set(DOXYGEN_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)

    # Generate doxygen configuration file
    configure_file(${DOXYGEN_CONFIG_FILE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    # Add custom target to generate documentation
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message("Doxygen needs to be installed to generate the documentation")
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OpenMP_FLAG "-fopenmp")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(OpenMP_FLAG "-qopenmp")
endif()

# Debug and Release configurations
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Configure the compiler and its properties for release or debug:
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ${OpenMP_FLAG}") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g" )
    add_definitions(-DDEBUG)
else()
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native ${OpenMP_FLAG}") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mavx2")
	add_definitions(-DNDEBUG)
endif()

#target_compile_options(GeometryLib PUBLIC ${OpenMP_FLAG})
target_compile_options(GeometryLib PUBLIC)