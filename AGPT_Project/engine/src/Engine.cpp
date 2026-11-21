#include <xe/Engine.h>

#include <algorithm>
#include <filesystem>
#include <SDL3/SDL.h>
#include <xe/InputManager.h>
#include <xe/ResourceManager.h>
#include <xe/Window.h>
#include "OpenGLRenderer.h"
#include "SdlWrappers.h"
#include <xe/Level.h>


namespace {

    // Resolves "<executable folder>/shaders" so shaders load regardless of working directory
    std::string defaultShaderDir() {
        const char* base = SDL_GetBasePath();  // Owned by SDL3; do not free
        return (std::filesystem::path(base ? base : "./") / "shaders").string();
    }

}  // namespace

namespace xe {

    // Members are constructed top-to-bottom and destroyed bottom-to-top automatically:
    // pending/level -> input -> resources -> renderer -> context -> window -> sdl.
    struct Engine::Impl {
        explicit Impl(const EngineConfig& c)
            : config(c),
            window(c.title, c.width, c.height),
            context(static_cast<SDL_Window*>(window.nativeHandle())),
            renderer(static_cast<SDL_Window*>(window.nativeHandle()), c.width, c.height),
            resources(renderer, c.assetsDir,
                c.shadersDir.empty() ? defaultShaderDir() : c.shadersDir) {
            renderer.initialize(resources);
        }

        EngineConfig config;
        detail::SdlSystem sdl;
        Window window;
        detail::GlContext context;
        detail::OpenGLRenderer renderer;   // Direct member: no extra unique_ptr needed
        ResourceManager resources;         // Direct member: no extra unique_ptr needed
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
    ResourceManager& Engine::resources() { return impl_->resources; }
    Renderer& Engine::renderer() { return impl_->renderer; }
    Window& Engine::window() { return impl_->window; }
    void Engine::setWindowTitle(const std::string& title) { impl_->window.setTitle(title); }

    int Engine::run() {
        Impl& app = *impl_;
        const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
        const double fixedStep = std::max(1e-4, static_cast<double>(app.config.fixedTimeStep));
        Uint64 prevTicks = SDL_GetPerformanceCounter();
        double accumulator = 0.0;

        while (app.running) {
            // 1. Safe level swap between frames
            if (app.pending) {
                app.level.reset();
                app.level = std::move(app.pending);
                app.level->onLoad();
            }

            // 2. Poll OS & input events
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) app.running = false;
                app.input.handleNativeEvent(&e);
            }
            app.input.update();

            // 3. Measure elapsed frame time (clamped to 0.25s to avoid physics spiral-of-death)
            const Uint64 now = SDL_GetPerformanceCounter();
            const double dt = std::min(0.25, static_cast<double>(now - prevTicks) / freq);
            prevTicks = now;

            if (!app.level) continue;

            // 4. Update game logic & step Box2D at a fixed rate
            app.level->update(static_cast<float>(dt));
            accumulator += dt;
            while (accumulator >= fixedStep) {
                app.level->fixedUpdate(static_cast<float>(fixedStep));
                accumulator -= fixedStep;
            }
            app.level->cleanup();

            // 5. Render finalized frame
            app.renderer.beginFrame(app.config.clearColor);
            app.level->render(app.renderer);
            app.renderer.endFrame();
        }

        return 0;
    }

}  // namespace xe