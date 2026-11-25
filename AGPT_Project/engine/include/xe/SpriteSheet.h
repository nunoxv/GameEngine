#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <xe/Texture.h>

namespace xe {

    // Wraps a single Texture divided into a uniform grid of animation tiles (columns x rows).
    // Selects frames by computing normalized UV sub-rectangles rather than creating separate textures.
    class SpriteSheet {
    public:
        SpriteSheet(std::shared_ptr<Texture> texture, int columns, int rows);

        // Helper factory when tile pixel dimensions are known (e.g., 64x64) instead of grid counts
        static std::shared_ptr<SpriteSheet> fromFrameSize(std::shared_ptr<Texture> texture,
            int frameWidth, int frameHeight);

        glm::vec4 frameUV(int frame) const;   // Normalized (u0, v0, u1, v1) rectangle in [0, 1]
        glm::vec2 frameSize() const;          // Pixel dimensions (width, height) of a single tile

        int frameCount() const { return columns_ * rows_; }
        const Texture& texture() const { return *texture_; }

    private:
        std::shared_ptr<Texture> texture_;
        int columns_;
        int rows_;
    };

}  // namespace xe