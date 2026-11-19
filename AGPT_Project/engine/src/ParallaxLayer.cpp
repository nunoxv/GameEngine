#include <xe/ParallaxLayer.h>

#include <cmath>
#include <xe/Renderer.h>

namespace xe {

ParallaxLayer::ParallaxLayer(Level& level, std::shared_ptr<Texture> texture, float speed,
                             int drawLayer, float tileScale)
    : Actor(level), texture_(std::move(texture)), speed_(speed), tileScale_(tileScale) {
    layer = drawLayer;
}

void ParallaxLayer::update(float dt) {
    const float tileH = texture_->height() * tileScale_;
    offset_ = std::fmod(offset_ + speed_ * dt, tileH);   // keeps floats small
}

void ParallaxLayer::render(Renderer& renderer) {
    const glm::vec2 vp = renderer.viewportSize();
    const float tileW = texture_->width() * tileScale_;
    const float tileH = texture_->height() * tileScale_;
    // The UV window slides over a GL_REPEAT texture; content scrolls downwards.
    const float v0 = -offset_ / tileH;
    SpriteDraw d;
    d.texture = texture_.get();
    d.uv = {0.f, v0, vp.x / tileW, v0 + vp.y / tileH};
    d.position = vp * 0.5f;
    d.size = vp;
    renderer.drawSprite(d);
}

}  // namespace xe
