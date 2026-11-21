#pragma once

#include <memory>
#include <string>
#include <vector>

namespace xe {

    // Engine-defined input tokens so game code never includes SDL3 headers
    enum class Key { Left, Right, Up, Down, A, D, W, S, Space, Escape, Enter };
    enum class GamepadButton { South, Start };   // South = Xbox 'A' / PlayStation 'Cross (X)'
    enum class GamepadAxis { LeftX, LeftY };

    // Abstracts keyboard and gamepads into named actions ("fire") and axes ("move_x", "move_y").
    // Game logic queries high-level actions rather than specific hardware buttons.
    class InputManager {
    public:
        InputManager();
        ~InputManager();

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        // Maps a logical action name (e.g., "fire") to any combination of keys and gamepad buttons
        void bindAction(const std::string& action, std::vector<Key> keys,
            std::vector<GamepadButton> buttons);

        // Maps a logical axis name (e.g., "move_x") to negative/positive keys and a gamepad stick
        void bindAxis(const std::string& axis, std::vector<Key> negativeKeys,
            std::vector<Key> positiveKeys, GamepadAxis gamepadAxis);

        bool isDown(const std::string& action) const;       // True every frame the action is held
        bool wasPressed(const std::string& action) const;   // True only on the single frame it went down
        float axis(const std::string& axis) const;          // Normalized value in [-1.0, 1.0]

        // Called internally by Engine::run() during event polling and frame updates
        void handleNativeEvent(const void* sdlEvent);
        void update();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

}  // namespace xe