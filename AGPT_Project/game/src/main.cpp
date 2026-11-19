#include <exception>
#include <iostream>
#include <xe/Engine.h>
#include <xe/InputManager.h>
#include "XenonLevel.h"

int main() {
    try {
        xe::EngineConfig config;
        config.title = "Xenon clone";
        config.width = 600;
        config.height = 800;
        config.assetsDir = XENON_ASSETS_DIR;

        xe::Engine engine(config);

        // Game code only knows ACTIONS; keyboard and gamepad are bound here.
        using xe::GamepadAxis;
        using xe::GamepadButton;
        using xe::Key;
        auto& input = engine.input();
        input.bindAxis("move_x", {Key::Left, Key::A}, {Key::Right, Key::D}, GamepadAxis::LeftX);
        input.bindAxis("move_y", {Key::Up, Key::W}, {Key::Down, Key::S}, GamepadAxis::LeftY);
        input.bindAction("fire", {Key::Space}, {GamepadButton::South});   // Xbox A / PS X
        input.bindAction("quit", {Key::Escape}, {});

        engine.loadLevel<XenonLevel>();
        return engine.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
