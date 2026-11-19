cmake_minimum_required(VERSION 3.22)
project(XenonClone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 3.4 CONFIG REQUIRED)   # fails early if SDL is older than 3.4

# Two independent projects, built together for convenience.
add_subdirectory(engine)   # library
add_subdirectory(game)     # executable, links xe::xengine
