#pragma once
#include <random>
#include <xe/Level.h>
#include <xe/SpriteSheet.h>

class Ship;

class XenonLevel : public xe::Level {
public:
    struct Assets {
        std::shared_ptr<xe::SpriteSheet> ship, missile, explosion, loner, rusher, enemyShot;
    };

    using xe::Level::Level;
    void onLoad() override;
    void onUpdate(float dt) override;

    const Assets& assets() const { return assets_; }
    Ship* ship();                       // nullptr while dead
    void addScore(int points);
    void onShipDestroyed();

    float randomFloat(float lo, float hi);
    bool randomBool();

private:
    void spawnShip();
    void refreshTitle();

    Assets assets_;
    std::mt19937 rng_{std::random_device{}()};
    std::uint64_t shipId_ = 0;
    float lonerTimer_ = 2.f, rusherTimer_ = 1.f, respawnTimer_ = -1.f;
    int score_ = 0, lives_ = 3;
    bool titleDirty_ = true;
};
