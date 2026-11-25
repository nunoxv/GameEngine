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

    // Stateless 2D spatial hash: produces deterministic values per (x, y) pixel
    // so opposite tile edges wrap seamlessly when rendered with OpenGL GL_REPEAT.
    std::uint32_t hash2(std::uint32_t x, std::uint32_t y, std::uint32_t seed) {
        std::uint32_t h = x * 374761393u + y * 668265263u + seed * 2246822519u;
        h = (h ^ (h >> 13)) * 1274126177u;
        return h ^ (h >> 16);
    }

    // Procedural fallback layer 0 (opaque): deep-space backdrop with dim distant stars
    void fillDeepSpace(std::vector<unsigned char>& px) {
        for (int y = 0; y < 256; ++y) {
            for (int x = 0; x < 256; ++x) {
                const std::uint32_t h = hash2(x, y, 1);
                const auto noise = static_cast<unsigned char>(h & 7);
                const size_t i = (static_cast<size_t>(y) * 256 + x) * 4;

                px[i + 0] = static_cast<unsigned char>(4 + noise);
                px[i + 1] = static_cast<unsigned char>(6 + noise);
                px[i + 2] = static_cast<unsigned char>(22 + noise * 2);
                px[i + 3] = 255;

                if ((h >> 8) % 350 == 0) {
                    px[i + 0] = px[i + 1] = px[i + 2] = static_cast<unsigned char>(90 + (h >> 20) % 70);
                }
            }
        }
    }

    // Procedural fallback layer 1 (transparent): sparse bright stars scrolling faster in front
    void fillNearStars(std::vector<unsigned char>& px) {
        for (int y = 0; y < 256; ++y) {
            for (int x = 0; x < 256; ++x) {
                const std::uint32_t h = hash2(x, y, 7);
                if ((h >> 8) % 260 == 0) {
                    const size_t i = (static_cast<size_t>(y) * 256 + x) * 4;
                    px[i + 0] = px[i + 1] = px[i + 2] = static_cast<unsigned char>(190 + (h >> 20) % 65);
                    px[i + 3] = 255;
                }
            }
        }
    }

}  // namespace

void XenonLevel::onLoad() {
    auto& res = engine().resources();

    // Helper to load a BMP via ResourceManager and slice it into a SpriteSheet grid
    auto loadSheet = [&](const SheetInfo& info) {
        return std::make_shared<xe::SpriteSheet>(res.texture(info.file), info.cols, info.rows);
        };

    assets_.ship = loadSheet(kShipSheet);
    assets_.missile = loadSheet(kMissileSheet);
    assets_.loner = loadSheet(kLonerSheet);
    assets_.rusher = loadSheet(kRusherSheet);
    assets_.enemyShot = loadSheet(kEnemyShotSheet);
    assets_.explosion = xe::SpriteSheet::fromFrameSize(
        res.texture(kExplosionFile), kExplosionFrameSize, kExplosionFrameSize);

    // Two parallax layers scrolling at different speeds (40 px/s far vs. 130 px/s near).
    // Uses BMP files from assets/ if present, otherwise generates seamless procedural textures.
    const xe::TextureOptions repeatOpaque{/*repeat=*/true, /*magentaColorKey=*/false };
    const xe::TextureOptions repeatKeyed{/*repeat=*/true, /*magentaColorKey=*/true };

    auto bg0 = res.assetExists(kBackground0File)
        ? res.texture(kBackground0File, repeatOpaque)
        : res.generatedTexture("bg0", 256, 256, fillDeepSpace, repeatOpaque);

    auto bg1 = res.assetExists(kBackground1File)
        ? res.texture(kBackground1File, repeatKeyed)
        : res.generatedTexture("bg1", 256, 256, fillNearStars, repeatKeyed);

    spawn<xe::ParallaxLayer>(bg0, 40.f, zorder::Back0, 2.f);
    spawn<xe::ParallaxLayer>(bg1, 130.f, zorder::Back1, 2.f);

    spawnShip();
    refreshTitle();
}

void XenonLevel::spawnShip() {
    const glm::vec2 vp = engine().renderer().viewportSize();
    Ship* s = spawn<Ship>(glm::vec2(vp.x * 0.5f, vp.y - 100.f));
    shipId_ = s->id();
}

Ship* XenonLevel::ship() {
    // Returns nullptr safely if the ship has been destroyed and removed from the level
    return dynamic_cast<Ship*>(find(shipId_));
}

void XenonLevel::onUpdate(float dt) {
    if (engine().input().wasPressed("quit")) {
        engine().quit();
        return;
    }

    const glm::vec2 vp = engine().renderer().viewportSize();

    // Periodically spawn Loners entering from either the left or right edge
    lonerTimer_ -= dt;
    if (lonerTimer_ <= 0.f) {
        lonerTimer_ = randomFloat(3.5f, 6.f);
        spawn<Loner>(randomFloat(60.f, vp.y * 0.4f), randomBool());
    }

    // Periodically spawn Rushers diving vertically from the top edge
    rusherTimer_ -= dt;
    if (rusherTimer_ <= 0.f) {
        rusherTimer_ = randomFloat(1.2f, 2.6f);
        spawn<Rusher>(randomFloat(40.f, vp.x - 40.f));
    }

    // Handle delayed player respawn after death
    if (respawnTimer_ > 0.f) {
        respawnTimer_ -= dt;
        if (respawnTimer_ <= 0.f) {
            spawnShip();
        }
    }
}

void XenonLevel::addScore(int points) {
    score_ += points;
    refreshTitle();
}

void XenonLevel::onShipDestroyed() {
    if (--lives_ <= 0) {
        // Reset run on Game Over
        lives_ = 3;
        score_ = 0;
    }
    respawnTimer_ = 2.f;
    refreshTitle();
}

void XenonLevel::refreshTitle() {
    engine().setWindowTitle("Xenon 2000 | Score " + std::to_string(score_) +
        " | Lives " + std::to_string(lives_));
}

float XenonLevel::randomFloat(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng_);
}

bool XenonLevel::randomBool() {
    return randomFloat(0.f, 1.f) < 0.5f;
}