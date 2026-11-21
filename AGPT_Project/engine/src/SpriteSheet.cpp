#include <xe/SpriteSheet.h>

#include <algorithm>
#include <iostream>
#include <utility>

namespace xe {

    SpriteSheet::SpriteSheet(std::shared_ptr<Texture> texture, int columns, int rows)
        : texture_(std::move(texture)),
        columns_(std::max(1, columns)),
        rows_(std::max(1, rows)) {
        // Warn during development if the BMP dimensions don't divide evenly into the requested grid
        if (texture_ && (texture_->width() % columns_ != 0 || texture_->height() % rows_ != 0)) {
            std::cerr << "[xe] warning: texture " << texture_->width() << "x" << texture_->height()
                << " is not divisible by a " << columns_ << "x" << rows_ << " frame grid\n";
        }
    }

    std::shared_ptr<SpriteSheet> SpriteSheet::fromFrameSize(std::shared_ptr<Texture> texture,
        int frameWidth, int frameHeight) {
        const int cols = texture ? texture->width() / std::max(1, frameWidth) : 1;
        const int rows = texture ? texture->height() / std::max(1, frameHeight) : 1;
        return std::make_shared<SpriteSheet>(std::move(texture), cols, rows);
    }

    glm::vec4 SpriteSheet::frameUV(int frame) const {
        // Clamp frame index to valid grid bounds [0, columns * rows - 1]
        frame = std::clamp(frame, 0, frameCount() - 1);

        // Convert 1D frame index into 2D grid coordinates (row-major, left-to-right, top-to-bottom)
        const int col = frame % columns_;
        const int row = frame / columns_;

        // Return normalized [0, 1] UV bounds: (u0, v0) = top-left, (u1, v1) = bottom-right
        return {
            float(col) / columns_,
            float(row) / rows_,
            float(col + 1) / columns_,
            float(row + 1) / rows_
        };
    }

    glm::vec2 SpriteSheet::frameSize() const {
        if (!texture_) return { 0.f, 0.f };
        return { float(texture_->width()) / columns_, float(texture_->height()) / rows_ };
    }

}  // namespace xe