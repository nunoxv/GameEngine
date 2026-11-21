#include <xe/Window.h>

#include <stdexcept>
#include <SDL3/SDL.h>

namespace xe {

    Window::Window(const std::string& title, int w, int h) : width_(w), height_(h) {
        // Configure OpenGL 3.3 Core context attributes BEFORE creating the window
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // Create an OpenGL-enabled window (SDL3 takes width & height directly, no x/y params)
        window_ = SDL_CreateWindow(title.c_str(), w, h, SDL_WINDOW_OPENGL);
        if (!window_) {
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
        }
    }

    Window::~Window() {
        // RAII cleanup: releases the OS window automatically when Window goes out of scope
        if (window_) {
            SDL_DestroyWindow(window_);
        }
    }

    void Window::setTitle(const std::string& title) {
        SDL_SetWindowTitle(window_, title.c_str());
    }

}  // namespace xe