#include <xe/SpriteSheet.h>

#include <algorithm>
#include <iostream>

namespace xe {

SpriteSheet::SpriteSheet(std::shared_ptr<Texture> texture, int columns, int rows)
    : texture_(std::move(texture)), columns_(std::max(1, columns)), rows_(std::max(1, rows)) {
    if (texture_ && (texture_->width() % columns_ != 0 || texture_->height() % rows_ != 0)) {
        std::cerr << "[xe] warning: texture " << texture_->width() << "x" << texture_->height()
                  << " is not divisible by a " << columns_ << "x" << rows_ << " frame grid\n";
    }
}

std::shared_ptr<SpriteSheet> SpriteSheet::fromFrameSize(std::shared_ptr<Texture> texture,
                                                        int fw, int fh) {
    const int cols = texture->width() / std::max(1, fw);
    const int rows = texture->height() / std::max(1, fh);
    return std::make_shared<SpriteSheet>(std::move(texture), cols, rows);
}

glm::vec4 SpriteSheet::frameUV(int frame) const {
    frame = std::clamp(frame, 0, frameCount() - 1);
    const int c = frame % columns_, r = frame / columns_;
    return {float(c) / columns_, float(r) / rows_, float(c + 1) / columns_, float(r + 1) / rows_};
}

glm::vec2 SpriteSheet::frameSize() const {
    return {float(texture_->width()) / columns_, float(texture_->height()) / rows_};
}

}  // namespace xe
