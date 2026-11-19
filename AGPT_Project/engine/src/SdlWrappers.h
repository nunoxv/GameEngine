#pragma once
// PRIVATE header: RAII owners for SDL init and the OpenGL context (the window is xe::Window).
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>

namespace xe::detail {

class SdlSystem {
public:
    SdlSystem() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }
    ~SdlSystem() { SDL_Quit(); }
    SdlSystem(const SdlSystem&) = delete;
    SdlSystem& operator=(const SdlSystem&) = delete;
};

class GlContext {
public:
    explicit GlContext(SDL_Window* window) {
        context_ = SDL_GL_CreateContext(window);
        if (!context_)
            throw std::runtime_error(std::string("SDL_GL_CreateContext failed: ") + SDL_GetError());
        if (!SDL_GL_MakeCurrent(window, context_)) {
            SDL_GL_DestroyContext(context_);
            throw std::runtime_error(std::string("SDL_GL_MakeCurrent failed: ") + SDL_GetError());
        }
        auto loader = [](const char* name) -> void* {
            return reinterpret_cast<void*>(SDL_GL_GetProcAddress(name));
        };
        if (!gladLoadGLLoader(loader)) {
            SDL_GL_DestroyContext(context_);
            throw std::runtime_error("Failed to load OpenGL functions (GLAD)");
        }
        SDL_GL_SetSwapInterval(1);   // vsync
    }
    ~GlContext() {
        if (context_) SDL_GL_DestroyContext(context_);
    }
    GlContext(const GlContext&) = delete;
    GlContext& operator=(const GlContext&) = delete;

private:
    SDL_GLContext context_ = nullptr;
};

}  // namespace xe::detail
