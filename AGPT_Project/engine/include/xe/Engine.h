#pragma once
#include <memory>
#include <string>
#include <utility>
#include <glm/glm.hpp>
#include <xe/Level.h>

namespace xe {

class InputManager;
class Renderer;
class ResourceManager;
class Window;

struct EngineConfig {
    std::string title = "XE Game";
    int width = 600;
    int height = 800;
    float fixedTimeStep = 1.f / 60.f;   // Box2D step, independent of the frame rate
    std::string assetsDir = "assets";
    std::string shadersDir;             // empty => "<executable dir>/shaders"
    glm::vec4 clearColor{0.f, 0.f, 0.f, 1.f};
};

// Owns window, GL context, renderer, resources, input and the active level.
// Everything is released by destructors (reverse declaration order).
class Engine {
public:
    explicit Engine(const EngineConfig& config);
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // Takes effect at the start of the next loop iteration.
    void setLevel(std::unique_ptr<Level> level);
    template <class T, class... Args>
    void loadLevel(Args&&... args) {
        setLevel(std::make_unique<T>(*this, std::forward<Args>(args)...));
    }

    int run();
    void quit();

    InputManager& input();
    ResourceManager& resources();
    Renderer& renderer();
    Window& window();
    void setWindowTitle(const std::string& title);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace xe
