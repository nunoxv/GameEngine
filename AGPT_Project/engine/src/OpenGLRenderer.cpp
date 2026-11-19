#include "OpenGLRenderer.h"

#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <xe/ResourceManager.h>

namespace xe::detail {
namespace {

struct SurfaceDeleter {
    void operator()(SDL_Surface* s) const { SDL_DestroySurface(s); }
};
using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

}  // namespace

OpenGLRenderer::OpenGLRenderer(SDL_Window* window, int w, int h)
    : window_(window), width_(w), height_(h),
      // Orthographic projection: the game works in pixels, origin top-left, y down.
      projection_(glm::ortho(0.f, float(w), float(h), 0.f, -1.f, 1.f)) {}

void OpenGLRenderer::initialize(ResourceManager& resources) {
    shader_ = std::static_pointer_cast<GLShader>(resources.shader("sprite"));
    quad_ = std::make_unique<QuadMesh>();
}

std::shared_ptr<Shader> OpenGLRenderer::createShader(const std::string& vs, const std::string& fs) {
    return std::make_shared<GLShader>(vs, fs);
}

std::shared_ptr<Texture> OpenGLRenderer::createTextureFromPixels(int w, int h,
                                                                 const unsigned char* rgba,
                                                                 const TextureOptions& o) {
    return std::make_shared<GLTexture>(w, h, rgba, o.repeat);
}

std::shared_ptr<Texture> OpenGLRenderer::createTextureFromFile(const std::string& path,
                                                               const TextureOptions& o) {
    SurfacePtr loaded(SDL_LoadBMP(path.c_str()));
    if (!loaded)
        throw std::runtime_error("Cannot load BMP '" + path + "': " + SDL_GetError());
    SurfacePtr rgba(SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32));
    if (!rgba)
        throw std::runtime_error("Cannot convert BMP '" + path + "': " + SDL_GetError());

    const int w = rgba->w, h = rgba->h;
    std::vector<unsigned char> pixels(static_cast<size_t>(w) * h * 4);
    for (int y = 0; y < h; ++y) {
        std::memcpy(&pixels[static_cast<size_t>(y) * w * 4],
                    static_cast<const unsigned char*>(rgba->pixels) + static_cast<size_t>(y) * rgba->pitch,
                    static_cast<size_t>(w) * 4);
    }
    if (o.magentaColorKey) {   // BMP has no alpha channel: magenta (255,0,255) => transparent
        for (size_t i = 0; i < pixels.size(); i += 4) {
            if (pixels[i] == 255 && pixels[i + 1] == 0 && pixels[i + 2] == 255)
                pixels[i] = pixels[i + 1] = pixels[i + 2] = pixels[i + 3] = 0;
        }
    }
    return std::make_shared<GLTexture>(w, h, pixels.data(), o.repeat);
}

void OpenGLRenderer::beginFrame(const glm::vec4& c) {
    int pw = width_, ph = height_;
    SDL_GetWindowSizeInPixels(window_, &pw, &ph);
    glViewport(0, 0, pw, ph);
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_->use();
    shader_->setMat4("uProjection", projection_);
    shader_->setInt("uTexture", 0);
    quad_->bind();
}

void OpenGLRenderer::drawSprite(const SpriteDraw& s) {
    if (!s.texture) return;
    // Model matrix = T * R * S
    glm::mat4 model(1.f);
    model = glm::translate(model, glm::vec3(s.position, 0.f));
    model = glm::rotate(model, s.rotation, glm::vec3(0.f, 0.f, 1.f));
    model = glm::scale(model, glm::vec3(s.size * s.scale, 1.f));
    shader_->setMat4("uModel", model);
    shader_->setVec4("uUvRect", s.uv);
    shader_->setVec4("uTint", s.tint);
    static_cast<const GLTexture*>(s.texture)->bind(0);
    quad_->draw();
}

void OpenGLRenderer::endFrame() { SDL_GL_SwapWindow(window_); }

}  // namespace xe::detail
