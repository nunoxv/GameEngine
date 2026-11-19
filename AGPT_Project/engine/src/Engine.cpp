#include <xe/Engine.h>

#include <algorithm>
#include <xe/InputManager.h>
#include <xe/ResourceManager.h>
#include <xe/Window.h>
#include <SDL3/SDL.h>
#include <filesystem>
#include "OpenGLRenderer.h"
#include "SdlWrappers.h"

namespace {
// Default shader location: a "shaders" folder next to the executable
// (the game's CMake copies it there with xengine_copy_shaders()).
std::string defaultShaderDir() {
    const char* base = SDL_GetBasePath();   // owned by SDL
    return (std::filesystem::path(base ? base : "./") / "shaders").string();
}
}  // namespace

namespace xe {

// Members are destroyed in reverse order: levels (and their textures) first, then
// resources, renderer, GL context, window and finally SDL itself. Nothing is freed by hand.
struct Engine::Impl {
    explicit Impl(const EngineConfig& c)
        : config(c),
          window(c.title, c.width, c.height),
          context(static_cast<SDL_Window*>(window.nativeHandle())),
          renderer(std::make_unique<detail::OpenGLRenderer>(static_cast<SDL_Window*>(window.nativeHandle()), c.width, c.height)),
          resources(std::make_unique<ResourceManager>(
              *renderer, c.assetsDir, c.shadersDir.empty() ? defaultShaderDir() : c.shadersDir)) {
        renderer->initialize(*resources);
    }

    void applyPendingLevel() {
        if (!pending) return;
        level.reset();                    // old actors/textures released first
        level = std::move(pending);
        level->onLoad();
    }

    EngineConfig config;
    detail::SdlSystem sdl;
    Window window;
    detail::GlContext context;
    std::unique_ptr<detail::OpenGLRenderer> renderer;
    std::unique_ptr<ResourceManager> resources;
    InputManager input;
    std::unique_ptr<Level> level;
    std::unique_ptr<Level> pending;
    bool running = true;
};

Engine::Engine(const EngineConfig& config) : impl_(std::make_unique<Impl>(config)) {}
Engine::~Engine() = default;

void Engine::setLevel(std::unique_ptr<Level> level) { impl_->pending = std::move(level); }
void Engine::quit() { impl_->running = false; }
InputManager& Engine::input() { return impl_->input; }
ResourceManager& Engine::resources() { return *impl_->resources; }
Renderer& Engine::renderer() { return *impl_->renderer; }
void Engine::setWindowTitle(const std::string& t) { impl_->window.setTitle(t); }
Window& Engine::window() { return impl_->window; }

int Engine::run() {
    Impl& I = *impl_;
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
    Uint64 previous = SDL_GetPerformanceCounter();
    double accumulator = 0.0;
    const double step = I.config.fixedTimeStep;

    while (I.running) {
        I.applyPendingLevel();

        // 1. Events -> InputManager
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) I.running = false;
            I.input.handleNativeEvent(&e);
        }
        I.input.update();

        const Uint64 now = SDL_GetPerformanceCounter();
        const double dt = std::min(0.25, static_cast<double>(now - previous) / freq);
        previous = now;

        if (!I.level) continue;

        // 2. UPDATE: all actors (input, logic), then physics at a FIXED time step
        I.level->update(static_cast<float>(dt));
        accumulator += dt;
        while (accumulator >= step) {
            I.level->fixedUpdate(static_cast<float>(step));
            accumulator -= step;
        }
        I.level->cleanup();

        // 3. RENDER: only after every actor has been updated
        I.renderer->beginFrame(I.config.clearColor);
        I.level->render(*I.renderer);
        I.renderer->endFrame();
    }
    return 0;
}

}  // namespace xe
