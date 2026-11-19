#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <xe/Texture.h>

namespace xe {

// A regular grid of frames inside ONE texture. Frames are addressed by UV rectangle,
// never by separate textures.
class SpriteSheet {
public:
    SpriteSheet(std::shared_ptr<Texture> texture, int columns, int rows);
    static std::shared_ptr<SpriteSheet> fromFrameSize(std::shared_ptr<Texture> texture,
                                                      int frameWidth, int frameHeight);

    glm::vec4 frameUV(int frame) const;   // u0, v0, u1, v1
    glm::vec2 frameSize() const;          // pixels
    int frameCount() const { return columns_ * rows_; }
    int columns() const { return columns_; }
    int rows() const { return rows_; }
    const Texture& texture() const { return *texture_; }

private:
    std::shared_ptr<Texture> texture_;
    int columns_, rows_;
};

}  // namespace xe
