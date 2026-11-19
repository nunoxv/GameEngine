#include <xe/Window.h>

#include <SDL3/SDL.h>
#include <stdexcept>

namespace xe {

struct Window::Impl {
    SDL_Window* window = nullptr;
};

Window::Window(const std::string& title, int w, int h)
    : impl_(std::make_unique<Impl>()), width_(w), height_(h) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    impl_->window = SDL_CreateWindow(title.c_str(), w, h, SDL_WINDOW_OPENGL);
    if (!impl_->window)
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
}

Window::~Window() {
    if (impl_ && impl_->window) SDL_DestroyWindow(impl_->window);
}

void Window::setTitle(const std::string& title) { SDL_SetWindowTitle(impl_->window, title.c_str()); }
void* Window::nativeHandle() const { return impl_->window; }

}  // namespace xe
