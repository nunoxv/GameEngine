#include <xe/InputManager.h>

#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace xe {
namespace {

struct GamepadDeleter {
    void operator()(SDL_Gamepad* g) const { SDL_CloseGamepad(g); }
};
using GamepadPtr = std::unique_ptr<SDL_Gamepad, GamepadDeleter>;

SDL_Scancode toScancode(Key k) {
    switch (k) {
        case Key::Left: return SDL_SCANCODE_LEFT;
        case Key::Right: return SDL_SCANCODE_RIGHT;
        case Key::Up: return SDL_SCANCODE_UP;
        case Key::Down: return SDL_SCANCODE_DOWN;
        case Key::A: return SDL_SCANCODE_A;
        case Key::D: return SDL_SCANCODE_D;
        case Key::W: return SDL_SCANCODE_W;
        case Key::S: return SDL_SCANCODE_S;
        case Key::Space: return SDL_SCANCODE_SPACE;
        case Key::Escape: return SDL_SCANCODE_ESCAPE;
        case Key::Enter: return SDL_SCANCODE_RETURN;
    }
    return SDL_SCANCODE_UNKNOWN;
}

SDL_GamepadButton toButton(GamepadButton b) {
    switch (b) {
        case GamepadButton::South: return SDL_GAMEPAD_BUTTON_SOUTH;   // Xbox A / PS Cross
        case GamepadButton::Start: return SDL_GAMEPAD_BUTTON_START;
    }
    return SDL_GAMEPAD_BUTTON_INVALID;
}

constexpr float kDeadzone = 0.2f;

}  // namespace

struct InputManager::Impl {
    struct Action {
        std::vector<Key> keys;
        std::vector<GamepadButton> buttons;
        bool down = false, previous = false;
    };
    struct Axis {
        std::vector<Key> negative, positive;
        GamepadAxis pad = GamepadAxis::LeftX;
        float value = 0.f;
    };
    std::unordered_map<std::string, Action> actions;
    std::unordered_map<std::string, Axis> axes;
    std::unordered_map<SDL_JoystickID, GamepadPtr> pads;   // closed automatically

    void open(SDL_JoystickID id) {
        if (pads.count(id)) return;
        if (SDL_Gamepad* g = SDL_OpenGamepad(id)) pads.emplace(id, GamepadPtr(g));
    }
};

InputManager::InputManager() : impl_(std::make_unique<Impl>()) {
    int count = 0;
    if (SDL_JoystickID* ids = SDL_GetGamepads(&count)) {
        for (int i = 0; i < count; ++i) impl_->open(ids[i]);
        SDL_free(ids);
    }
}
InputManager::~InputManager() = default;

void InputManager::bindAction(const std::string& name, std::vector<Key> keys,
                              std::vector<GamepadButton> buttons) {
    auto& a = impl_->actions[name];
    a.keys = std::move(keys);
    a.buttons = std::move(buttons);
}

void InputManager::bindAxis(const std::string& name, std::vector<Key> neg, std::vector<Key> pos,
                            GamepadAxis pad) {
    auto& a = impl_->axes[name];
    a.negative = std::move(neg);
    a.positive = std::move(pos);
    a.pad = pad;
}

bool InputManager::isDown(const std::string& n) const {
    auto it = impl_->actions.find(n);
    return it != impl_->actions.end() && it->second.down;
}
bool InputManager::wasPressed(const std::string& n) const {
    auto it = impl_->actions.find(n);
    return it != impl_->actions.end() && it->second.down && !it->second.previous;
}
float InputManager::axis(const std::string& n) const {
    auto it = impl_->axes.find(n);
    return it == impl_->axes.end() ? 0.f : it->second.value;
}

void InputManager::handleNativeEvent(const void* native) {
    const SDL_Event& e = *static_cast<const SDL_Event*>(native);
    if (e.type == SDL_EVENT_GAMEPAD_ADDED) impl_->open(e.gdevice.which);
    else if (e.type == SDL_EVENT_GAMEPAD_REMOVED) impl_->pads.erase(e.gdevice.which);
}

void InputManager::update() {
    const bool* keys = SDL_GetKeyboardState(nullptr);
    auto keyDown = [&](Key k) { return keys[toScancode(k)]; };

    for (auto& [name, a] : impl_->actions) {
        a.previous = a.down;
        a.down = std::any_of(a.keys.begin(), a.keys.end(), keyDown);
        for (auto& [id, pad] : impl_->pads)
            for (GamepadButton b : a.buttons)
                if (SDL_GetGamepadButton(pad.get(), toButton(b))) a.down = true;
    }

    for (auto& [name, ax] : impl_->axes) {
        float v = 0.f;
        if (std::any_of(ax.negative.begin(), ax.negative.end(), keyDown)) v -= 1.f;
        if (std::any_of(ax.positive.begin(), ax.positive.end(), keyDown)) v += 1.f;
        for (auto& [id, pad] : impl_->pads) {
            const bool horizontal = ax.pad == GamepadAxis::LeftX;
            const float stick = SDL_GetGamepadAxis(pad.get(), horizontal ? SDL_GAMEPAD_AXIS_LEFTX
                                                                          : SDL_GAMEPAD_AXIS_LEFTY) / 32767.f;
            if (std::fabs(stick) > kDeadzone) v += stick;
            const auto neg = horizontal ? SDL_GAMEPAD_BUTTON_DPAD_LEFT : SDL_GAMEPAD_BUTTON_DPAD_UP;
            const auto pos = horizontal ? SDL_GAMEPAD_BUTTON_DPAD_RIGHT : SDL_GAMEPAD_BUTTON_DPAD_DOWN;
            if (SDL_GetGamepadButton(pad.get(), neg)) v -= 1.f;
            if (SDL_GetGamepadButton(pad.get(), pos)) v += 1.f;
        }
        ax.value = std::clamp(v, -1.f, 1.f);
    }
}

}  // namespace xe
