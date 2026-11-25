#pragma once

#include <string>

// Forward declaration keeps <SDL3/SDL.h> out of public engine headers
struct SDL_Window;

namespace xe {

    // RAII wrapper around an OpenGL-capable OS window.
    // Acquires the SDL_Window in the constructor and destroys it in the destructor.
    class Window {
    public:
        Window(const std::string& title, int width, int height);
        ~Window();

        // Non-copyable because this object uniquely owns an OS window handle
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void setTitle(const std::string& title);

        // Exposes the underlying SDL_Window* as an opaque pointer for internal engine classes
        void* nativeHandle() const { return window_; }

    private:
        SDL_Window* window_ = nullptr;
        int width_;
        int height_;
    };

}  // namespace xe