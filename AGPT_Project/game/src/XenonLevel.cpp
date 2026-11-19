#include "XenonLevel.h"

#include <cstdint>
#include <string>
#include <vector>
#include <xe/Engine.h>
#include <xe/InputManager.h>
#include <xe/ParallaxLayer.h>
#include <xe/Renderer.h>
#include <xe/ResourceManager.h>
#include "Actors.h"
#include "GameAssets.h"

namespace {

std::uint32_t hash2(std::uint32_t x, std::uint32_t y, std::uint32_t seed) {
    std::uint32_t h = x * 374761393u + y * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

// Tileable procedural backgrounds (used when no BMP art is provided).
void fillDeepSpace(std::vector<unsigned char>& px) {   // opaque, dim
    for (int y = 0; y < 256; ++y)
        for (int x = 0; x < 256; ++x) {
            const std::uint32_t h = hash2(x, y, 1);
            unsigned char* p = &px[(static_cast<size_t>(y) * 256 + x) * 4];
            const unsigned char n = static_cast<unsigned char>(h & 7);
            p[0] = static_cast<unsigned char>(4 + n);
            p[1] = static_cast<unsigned char>(6 + n);
            p[2] = static_cast<unsigned char>(22 + n * 2);
            p[3] = 255;
            if ((h >> 8) % 350 == 0) p[0] = p[1] = p[2] = static_cast<unsigned char>(90 + (h >> 20) % 70);
        }
}
void fillNearStars(std::vector<unsigned char>& px) {   // transparent + bright stars
    for (int y = 0; y < 256; ++y)
        for (int x = 0; x < 256; ++x) {
            const std::uint32_t h = hash2(x, y, 7);
            unsigned char* p = &px[(static_cast<size_t>(y) * 256 + x) * 4];
            if ((h >> 8) % 260 == 0) {
                p[0] = p[1] = p[2] = static_cast<unsigned char>(190 + (h >> 20) % 65);
                p[3] = 255;
            }
        }
}

}  // namespace

void XenonLevel::onLoad() {
    auto& res = engine().resources();
    auto load = [&](const SheetInfo& s) {
        return std::make_shared<xe::SpriteSheet>(res.texture(s.file), s.cols, s.rows);
    };
    assets_.ship = load(kShipSheet);
    assets_.missile = load(kMissileSheet);
    assets_.loner = load(kLonerSheet);
    assets_.rusher = load(kRusherSheet);
    assets_.enemyShot = load(kEnemyShotSheet);
    assets_.explosion = xe::SpriteSheet::fromFrameSize(res.texture(kExplosionFile),
                                                       kExplosionFrameSize, kExplosionFrameSize);

    // Two parallax layers scrolling at different speeds.
    xe::TextureOptions opaque{true, false};
    xe::TextureOptions keyed{true, true};
    auto bg0 = res.assetExists(kBackground0File)
                   ? res.texture(kBackground0File, opaque)
                   : res.generatedTexture("bg0", 256, 256, fillDeepSpace, opaque);
    auto bg1 = res.assetExists(kBackground1File)
                   ? res.texture(kBackground1File, keyed)
                   : res.generatedTexture("bg1", 256, 256, fillNearStars, keyed);
    spawn<xe::ParallaxLayer>(bg0, 40.f, zorder::Back0, 2.f);
    spawn<xe::ParallaxLayer>(bg1, 130.f, zorder::Back1, 2.f);

    spawnShip();
}

void XenonLevel::spawnShip() {
    const glm::vec2 vp = engine().renderer().viewportSize();
    Ship* s = spawn<Ship>(glm::vec2(vp.x * 0.5f, vp.y - 100.f));
    shipId_ = s->id();
}

Ship* XenonLevel::ship() { return dynamic_cast<Ship*>(find(shipId_)); }

void XenonLevel::onUpdate(float dt) {
    auto& input = engine().input();
    if (input.wasPressed("quit")) engine().quit();

    const glm::vec2 vp = engine().renderer().viewportSize();
    lonerTimer_ -= dt;
    if (lonerTimer_ <= 0.f) {
        lonerTimer_ = randomFloat(3.5f, 6.f);
        spawn<Loner>(randomFloat(60.f, vp.y * 0.4f), randomBool());
    }
    rusherTimer_ -= dt;
    if (rusherTimer_ <= 0.f) {
        rusherTimer_ = randomFloat(1.2f, 2.6f);
        spawn<Rusher>(randomFloat(40.f, vp.x - 40.f));
    }
    if (respawnTimer_ > 0.f) {
        respawnTimer_ -= dt;
        if (respawnTimer_ <= 0.f) spawnShip();
    }
    if (titleDirty_) refreshTitle();
}

void XenonLevel::addScore(int points) {
    score_ += points;
    titleDirty_ = true;
}

void XenonLevel::onShipDestroyed() {
    if (--lives_ <= 0) {   // game over: restart
        lives_ = 3;
        score_ = 0;
    }
    respawnTimer_ = 2.f;
    titleDirty_ = true;
}

void XenonLevel::refreshTitle() {
    titleDirty_ = false;
    engine().setWindowTitle("Xenon clone | Score " + std::to_string(score_) + " | Lives " +
                            std::to_string(lives_));
}

float XenonLevel::randomFloat(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng_);
}
bool XenonLevel::randomBool() { return std::uniform_int_distribution<int>(0, 1)(rng_) == 1; }
