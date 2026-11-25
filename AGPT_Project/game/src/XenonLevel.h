#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <xe/Level.h>
#include <xe/SpriteSheet.h>

class Ship;

// Main playable scene for the Xenon 2000 clone.
// Loads sprite sheets, sets up two parallax background layers, spawns enemy waves,
// and tracks player score, lives, and ship respawns.
class XenonLevel : public xe::Level {
public:
    // Shared sprite sheets cached by ResourceManager and reused by all spawned actors
    struct Assets {
        std::shared_ptr<xe::SpriteSheet> ship;
        std::shared_ptr<xe::SpriteSheet> missile;
        std::shared_ptr<xe::SpriteSheet> explosion;
        std::shared_ptr<xe::SpriteSheet> loner;
        std::shared_ptr<xe::SpriteSheet> rusher;
        std::shared_ptr<xe::SpriteSheet> enemyShot;
    };

    using xe::Level::Level;

    void onLoad() override;
    void onUpdate(float dt) override;

    const Assets& assets() const { return assets_; }
    Ship* ship();                       // Safe lookup: returns nullptr while the ship is dead
    void addScore(int points);
    void onShipDestroyed();

    float randomFloat(float lo, float hi);
    bool randomBool();

private:
    void spawnShip();
    void refreshTitle();

    Assets assets_;
    std::mt19937 rng_{ std::random_device{}() };
    std::uint64_t shipId_ = 0;          // Safe ID handle instead of a raw Ship* pointer
    float lonerTimer_ = 2.f;
    float rusherTimer_ = 1.f;
    float respawnTimer_ = -1.f;
    int score_ = 0;
    int lives_ = 3;
};