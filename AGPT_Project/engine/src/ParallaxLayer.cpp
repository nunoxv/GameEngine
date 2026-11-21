#include <xe/ParallaxLayer.h>

#include <cmath>
#include <utility>
#include <xe/Renderer.h>

namespace xe {

    ParallaxLayer::ParallaxLayer(Level& level, std::shared_ptr<Texture> texture, float speed,
        int drawLayer, float tileScale)
        : Actor(level), texture_(std::move(texture)), speed_(speed), tileScale_(tileScale) {
        // Determines back-to-front draw order in Level::render() (e.g., -20 for far stars, -10 for near dust)
        layer = drawLayer;
    }

    void ParallaxLayer::update(float dt) {
        if (!texture_) return;

        const float tileH = texture_->height() * tileScale_;
        if (tileH <= 0.f) return;

        // Wrap offset_ within [0, tileH) using fmod so UV precision never degrades over long play sessions
        offset_ = std::fmod(offset_ + speed_ * dt, tileH);
    }

    void ParallaxLayer::render(Renderer& renderer) {
        if (!texture_) return;

        const glm::vec2 vp = renderer.viewportSize();
        const float tileW = texture_->width() * tileScale_;
        const float tileH = texture_->height() * tileScale_;
        if (tileW <= 0.f || tileH <= 0.f) return;

        // Convert pixel offset into normalized UV space [0, 1].
        // Negating offset_ shifts the texture window upward so the background visibly scrolls downward.
        const float v0 = -offset_ / tileH;

        SpriteDraw d;
        d.texture = texture_.get();
        d.uv = { 0.f, v0, vp.x / tileW, v0 + vp.y / tileH };  // (u0, v0, u1, v1) tiled across viewport
        d.position = vp * 0.5f;  // Centered on screen (quad vertices are centered around origin)
        d.size = vp;             // Stretched to cover the entire viewport
        renderer.drawSprite(d);
    }

}  // namespace xe