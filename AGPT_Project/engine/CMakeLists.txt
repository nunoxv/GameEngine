cmake_minimum_required(VERSION 3.22)
project(XEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 3.4 CONFIG REQUIRED)  
find_package(box2d CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_library(xengine STATIC
  src/Actor.cpp
  src/Animation.cpp
  src/Engine.cpp
  src/InputManager.cpp
  src/Level.cpp
  src/OpenGLRenderer.cpp
  src/ParallaxLayer.cpp
  src/Physics.cpp
  src/ResourceManager.cpp
  src/SpriteSheet.cpp
  src/Window.cpp
)
add_library(xe::xengine ALIAS xengine)

# Public headers contain NO SDL3 / OpenGL / Box2D includes (only GLM + std).
target_include_directories(xengine
  PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:include>
  PRIVATE src)
target_link_libraries(xengine PUBLIC glm::glm PRIVATE SDL3::SDL3 box2d::box2d glad::glad)
if(MSVC)
  target_compile_options(xengine PRIVATE /W4)
else()
  target_compile_options(xengine PRIVATE -Wall -Wextra)
endif()

# Used by games that add this directory with add_subdirectory() (same variable the
# installed package config provides).
set(XENGINE_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders" CACHE INTERNAL "")
include(cmake/xengineHelpers.cmake)