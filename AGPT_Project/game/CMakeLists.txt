cmake_minimum_required(VERSION 3.22)
project(XenonGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Built from the repository root, the engine target already exists.
# Built on its own, the game uses the installed engine package.
if(NOT TARGET xe::xengine)
  find_package(xengine CONFIG REQUIRED)
endif()

add_executable(xenon
  src/main.cpp
  src/Actors.cpp
  src/XenonLevel.cpp
)
# The game only sees the engine's public headers: no SDL3 / OpenGL / Box2D include paths.
target_link_libraries(xenon PRIVATE xe::xengine)
target_compile_definitions(xenon PRIVATE XENON_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../assets")
xengine_copy_shaders(xenon)
if(MSVC)
  target_compile_options(xenon PRIVATE /W4)
else()
  target_compile_options(xenon PRIVATE -Wall -Wextra)
endif()
