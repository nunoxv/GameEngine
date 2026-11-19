#pragma once
#include <memory>
#include <xe/Actor.h>
#include <xe/Texture.h>

namespace xe {

// Full-screen scrolling background. The texture must be created with
// TextureOptions::repeat = true; scrolling moves the UV window (GL_REPEAT).
class ParallaxLayer : public Actor {
public:
    ParallaxLayer(Level& level, std::shared_ptr<Texture> texture, float speedPixelsPerSecond,
                  int drawLayer, float tileScale = 1.f);
    void update(float dt) override;
    void render(Renderer& renderer) override;

private:
    std::shared_ptr<Texture> texture_;
    float speed_;
    float tileScale_;
    float offset_ = 0.f;
};

}  // namespace xe
