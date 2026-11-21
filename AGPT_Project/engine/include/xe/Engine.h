#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace xe {

    class InputManager;
    class Level;
    class Renderer;
    class ResourceManager;
    class Window;

    struct EngineConfig {
        std::string title = "XE Game";
        int width = 600;
        int height = 800;
        float fixedTimeStep = 1.f / 60.f;   // Fixed Box2D step (60 Hz), independent of render FPS
        std::string assetsDir = "assets";
        std::string shadersDir;             // Empty => "<executable dir>/shaders"
        glm::vec4 clearColor{ 0.f, 0.f, 0.f, 1.f };
    };

    // Top-level engine facade. Hides SDL3, OpenGL, and Box2D behind Pimpl so game code
    // stays 100% decoupled from third-party libraries, and frees everything via RAII.
    class Engine {
    public:
        explicit Engine(const EngineConfig& config);
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        // Swaps in a new level safely at the start of the next frame
        void setLevel(std::unique_ptr<Level> level);

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