#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <xe/Texture.h>

namespace xe {

// Everything the renderer needs to know to draw one sprite. Actors describe WHAT to
// draw; the back end decides HOW.
struct SpriteDraw {
    const Texture* texture = nullptr;
    glm::vec4 uv{0.f, 0.f, 1.f, 1.f};  // u0, v0, u1, v1 (v grows downwards)
    glm::vec2 position{0.f};           // centre, in pixels
    glm::vec2 size{1.f};               // pixels
    float rotation = 0.f;              // radians
    glm::vec2 scale{1.f};
    glm::vec4 tint{1.f};
};

class Renderer {
public:
    virtual ~Renderer() = default;

    virtual std::shared_ptr<Shader> createShader(const std::string& vertexPath,
                                                 const std::string& fragmentPath) = 0;
    virtual std::shared_ptr<Texture> createTextureFromFile(const std::string& path,
                                                           const TextureOptions& options) = 0;
    virtual std::shared_ptr<Texture> createTextureFromPixels(int width, int height,
                                                             const unsigned char* rgba,
                                                             const TextureOptions& options) = 0;

    virtual void beginFrame(const glm::vec4& clearColor) = 0;
    virtual void drawSprite(const SpriteDraw& sprite) = 0;
    virtual void endFrame() = 0;

    virtual glm::vec2 viewportSize() const = 0;  // logical size in pixels
};

}  // namespace xe
