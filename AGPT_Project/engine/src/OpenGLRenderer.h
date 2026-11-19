#pragma once
// PRIVATE header: OpenGL 3.3 back end of xe::Renderer.
#include <memory>
#include <xe/Renderer.h>
#include "GLObjects.h"

struct SDL_Window;

namespace xe {
class ResourceManager;
namespace detail {

class OpenGLRenderer final : public Renderer {
public:
    OpenGLRenderer(SDL_Window* window, int logicalWidth, int logicalHeight);
    void initialize(ResourceManager& resources);   // needs a current GL context

    std::shared_ptr<Shader> createShader(const std::string& vertexPath,
                                         const std::string& fragmentPath) override;
    std::shared_ptr<Texture> createTextureFromFile(const std::string& path,
                                                   const TextureOptions& options) override;
    std::shared_ptr<Texture> createTextureFromPixels(int width, int height,
                                                     const unsigned char* rgba,
                                                     const TextureOptions& options) override;
    void beginFrame(const glm::vec4& clearColor) override;
    void drawSprite(const SpriteDraw& sprite) override;
    void endFrame() override;
    glm::vec2 viewportSize() const override { return {float(width_), float(height_)}; }

private:
    SDL_Window* window_;   // not owned
    int width_, height_;
    std::shared_ptr<GLShader> shader_;
    std::unique_ptr<QuadMesh> quad_;
    glm::mat4 projection_;
};

}  // namespace detail
}  // namespace xe
