#include <exception>
#include <iostream>
#include <memory>
#include <xe/Engine.h>
#include <xe/InputManager.h>
#include "XenonLevel.h"

int main() {
    try {
        // 1. Configure window, resolution, and asset directory (injected via CMake)
        xe::EngineConfig config;
        config.title = "Xenon 2000 Clone";
        config.width = 600;
        config.height = 800;
        config.assetsDir = XENON_ASSETS_DIR;

        // 2. Construct the engine on the stack; RAII guarantees automatic cleanup on exit
        xe::Engine engine(config);

        // 3. Bind hardware keys & gamepad controls to abstract game actions.
        // Gameplay actors (Spaceship, XenonLevel) only query "move_x", "move_y", "fire", and "quit".
        using xe::GamepadAxis;
        using xe::GamepadButton;
        using xe::Key;

        auto& input = engine.input();
        input.bindAxis("move_x", { Key::Left, Key::A }, { Key::Right, Key::D }, GamepadAxis::LeftX);
        input.bindAxis("move_y", { Key::Up, Key::W }, { Key::Down, Key::S }, GamepadAxis::LeftY);
        input.bindAction("fire", { Key::Space }, { GamepadButton::South }); // Xbox 'A' / PlayStation 'X'
        input.bindAction("quit", { Key::Escape }, { GamepadButton::Start });

        // 4. Load the initial level and start the fixed-timestep game loop
        engine.setLevel(std::make_unique<XenonLevel>(engine));
        return engine.run();

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}