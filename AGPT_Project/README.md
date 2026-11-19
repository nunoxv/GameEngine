# Xenon clone: own 2D engine (SDL3 + OpenGL 3.3 + Box2D)

Common Project 1 (ATGP + CG). Two projects: `engine/` (static library `xengine`) and `game/` (executable `xenon`).
The game includes **only** `xe/*.h`; no SDL3 / OpenGL / Box2D headers.

## Layout: two projects
```
engine/   XEngine library project (own CMakeLists, vcpkg.json, install/export rules)
game/     Xenon game project (own CMakeLists, vcpkg.json); links xe::xengine
assets/   Xenon 2000 BMPs
CMakeLists.txt   convenience root that builds both together
```

## Build (clean clone)
Requirements: CMake >= 3.21, a C++17 compiler, [vcpkg](https://vcpkg.io) with `VCPKG_ROOT` set.

**Option A: everything at once (simplest)**
```
cmake --preset default        # vcpkg installs sdl3, box2d, glm, glad from the root vcpkg.json
cmake --build build
./build/game/xenon            # Windows: build\game\Debug\xenon.exe
```
**Option B: as two separate projects**
```
cd engine
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_INSTALL_PREFIX=../install
cmake --build build && cmake --install build
cd ../game
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DCMAKE_PREFIX_PATH=../install
cmake --build build
./build/xenon
```
(On Windows with Visual Studio, add `--config Debug` to build/install and run `build\Debug\xenon.exe`.)
AddressSanitizer build of option A: `cmake --preset asan && cmake --build build-asan`.

## Assets
Copy the Xenon 2000 BMPs into `assets/`:
`Ship1.bmp missile.bmp explode64.bmp LonerA.bmp EnWeap6.bmp rusher.bmp`.
Magenta (255,0,255) becomes transparent at load time. Backgrounds are procedural unless you add
`bg_layer0.bmp` / `bg_layer1.bmp`.
**Frame grids** (columns x rows per sheet) are in `game/src/GameAssets.h`. They are best guesses:
open the BMPs and adjust if a sprite looks wrong (the engine warns on stderr if a sheet is not divisible).

## Controls
Arrows/WASD or left stick/D-pad to move, Space / Xbox A / PlayStation X to fire, Esc to quit.

## Architecture map
| Requirement | Where |
|---|---|
| RAII wrappers | `Window.*` (window), `SdlWrappers.h` (SDL init / GL context), `GLObjects.h` (texture, shader, VAO/VBO/EBO), `Physics.cpp` (b2World/b2Body), `InputManager.cpp` (gamepads) |
| Renderer abstraction | `xe/Renderer.h` (abstract) + `OpenGLRenderer.*` (back end) |
| Game loop | `Engine::run`: update all actors, fixed-step Box2D accumulator, then render |
| Input | `xe/InputManager.h`: actions and axes, keyboard + gamepad |
| Resource manager | `xe/ResourceManager.h`: texture/shader cache |
| Level/Actor ownership | `Level` owns `unique_ptr<Actor>`; deferred spawn/destroy; actors referenced by id |
| Animation | `xe/Animation.h` (clips, loop/one-shot), ship turning states in `Actors.cpp` |
| Box2D collisions | sensor shapes + category/mask filters, `Level::fixedUpdate` dispatches `onOverlap` |
| Shaders / UV frames / matrices | `shaders/sprite.*`, `SpriteSheet::frameUV`, `OpenGLRenderer::drawSprite` (ortho + T*R*S) |
| Parallax | `ParallaxLayer`: UV window over `GL_REPEAT` textures, two speeds |
