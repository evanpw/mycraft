cmake_minimum_required(VERSION 3.20)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(mycraft)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)

## Main Executable ##
add_executable(
    mycraft
    src/block_library.cpp
    src/chunk.cpp
    src/coordinate.cpp
    src/mesh.cpp
    src/perlin_noise.cpp
    src/ray_caster.cpp
    src/shaders.cpp
    src/camera.cpp
    src/chunk_manager.cpp
    src/cube.cpp
    src/mycraft.cpp
    src/player.cpp
    src/renderer.cpp
    src/textures.cpp
)

target_link_libraries(mycraft PRIVATE OpenGL::GL glfw glm::glm GLEW::GLEW PNG::PNG)
target_include_directories(mycraft PRIVATE h/)

target_compile_options(
    mycraft
    PRIVATE
    -Wall
    -Wextra
)
