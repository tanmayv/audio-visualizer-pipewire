cmake_minimum_required(VERSION 3.25)
project(AudioVisualizerProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages using pkg-config
find_package(PkgConfig REQUIRED)
pkg_search_module(PIPEWIRE REQUIRED libpipewire-0.3)
pkg_search_module(RAYLIB REQUIRED raylib)
pkg_search_module(FFTW REQUIRED fftw3)

# Add your source files from the src folder
file(GLOB SOURCES "src/*.cc")
file(GLOB HEADERS "src/*.h")

# Add executable and link libraries
add_executable(AudioVisualizer ${SOURCES})
target_link_libraries(AudioVisualizer ${PIPEWIRE_LIBRARIES} ${RAYLIB_LIBRARIES} ${FFTW_LIBRARIES})
include_directories(${PIPEWIRE_INCLUDE_DIRS} ${HEADERS} ${RAYLIB_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})

# Export compile_commands.json for IDEs
