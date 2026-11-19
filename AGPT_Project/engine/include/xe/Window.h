#pragma once
#include <memory>
#include <string>

namespace xe {

// OS window with an OpenGL-capable surface. RAII: created in the constructor,
// destroyed in the destructor. SDL stays hidden behind the pimpl.
class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void setTitle(const std::string& title);
    int width() const { return width_; }
    int height() const { return height_; }

    void* nativeHandle() const;   // engine internals (SDL_Window*)

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    int width_, height_;
};

}  // namespace xe
