cmake_minimum_required(VERSION 3.25)
project(AudioVisualizerProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -limgui")
# Find required packages using pkg-config
find_package(PkgConfig REQUIRED)
pkg_search_module(PIPEWIRE REQUIRED libpipewire-0.3)
pkg_search_module(RAYLIB REQUIRED raylib)
pkg_search_module(FFTW REQUIRED fftw3)

# Add your source files from the src folder
file(GLOB SOURCES "src/*.cc")
file(GLOB HEADERS "src/*.h")

file(GLOB RL_SOURCES "external/rlImGui/*.cpp")
file(GLOB RL_HEADERS "external/rlImGui/*.h")


# Add executable and link libraries
add_executable(AudioVisualizer ${SOURCES} ${HEADERS} ${RL_SOURCES} ${RL_HEADERS})
target_link_libraries(AudioVisualizer ${PIPEWIRE_LIBRARIES} ${RAYLIB_LIBRARIES} ${FFTW_LIBRARIES})
include_directories(${PIPEWIRE_INCLUDE_DIRS} ${HEADERS} ${RAYLIB_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/external/rlImGui")

# Export compile_commands.json for IDEs
