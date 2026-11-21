#pragma once

#include <memory>
#include <xe/Actor.h>
#include <xe/Texture.h>

namespace xe {

    // Full-screen vertically scrolling background layer.
    // Requires a texture loaded with TextureOptions::repeat = true (GL_REPEAT)
    // so UV coordinates outside [0, 1] wrap seamlessly without tearing or extra quads.
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
        float offset_ = 0.f;  // Accumulated vertical scroll distance in pixels
    };

}  // namespace xe