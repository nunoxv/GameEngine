# XEngine: 2D game engine library

Window, GL context, OpenGL renderer (behind the abstract `xe::Renderer`), `ResourceManager`,
`InputManager`, `Level`, `Actor`, `Animator`, `SpriteSheet`, `ParallaxLayer`, Box2D `PhysicsWorld`.
Public headers (`include/xe/`) never include SDL3, OpenGL or Box2D.

Standalone build + install:
```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_INSTALL_PREFIX=<install-dir>
cmake --build build
cmake --install build
```
Consume it from another project:
```cmake
find_package(xengine CONFIG REQUIRED)
target_link_libraries(mygame PRIVATE xe::xengine)
xengine_copy_shaders(mygame)
```
