#pragma once
#include <memory>
#include <string>
#include <vector>

namespace xe {

enum class Key { Left, Right, Up, Down, A, D, W, S, Space, Escape, Enter };
enum class GamepadButton { South, Start };   // South = Xbox A / PlayStation Cross (X)
enum class GamepadAxis { LeftX, LeftY };

// Game code asks for ACTIONS ("fire", "move_x"), never for keys or buttons.
class InputManager {
public:
    InputManager();
    ~InputManager();
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void bindAction(const std::string& action, std::vector<Key> keys,
                    std::vector<GamepadButton> buttons);
    void bindAxis(const std::string& axis, std::vector<Key> negativeKeys,
                  std::vector<Key> positiveKeys, GamepadAxis gamepadAxis);

    bool isDown(const std::string& action) const;
    bool wasPressed(const std::string& action) const;   // went down this frame
    float axis(const std::string& axis) const;          // -1 .. 1

    // Engine internals
    void handleNativeEvent(const void* sdlEvent);
    void update();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace xe
