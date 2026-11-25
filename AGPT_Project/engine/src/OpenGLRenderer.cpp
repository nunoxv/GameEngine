#include "OpenGLRenderer.h"

#include <stdexcept>
#include <vector>
#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <xe/ResourceManager.h>

namespace xe::detail {
    namespace {

        // RAII deleter so temporary SDL_Surface objects are always freed automatically
        struct SurfaceDeleter {
            void operator()(SDL_Surface* s) const { SDL_DestroySurface(s); }
        };
        using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

    }  // namespace

    OpenGLRenderer::OpenGLRenderer(SDL_Window* window, int w, int h)
        : window_(window),
        width_(w),
        height_(h),
        // Orthographic projection mapping [0, w] x [0, h] to OpenGL Normalized Device Coordinates.
        // Top-left is (0, 0) and +Y points downward to match 2D screen/sprite coordinates.
        projection_(glm::ortho(0.f, float(w), float(h), 0.f, -1.f, 1.f)) {
    }

    void OpenGLRenderer::initialize(ResourceManager& resources) {
        shader_ = std::static_pointer_cast<GLShader>(resources.shader("sprite"));
        quad_ = std::make_unique<QuadMesh>();

        // Enable standard alpha blending so transparent sprite regions show layers behind them
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    std::shared_ptr<Shader> OpenGLRenderer::createShader(const std::string& vertexPath,
        const std::string& fragmentPath) {
        return std::make_shared<GLShader>(vertexPath, fragmentPath);
    }

    std::shared_ptr<Texture> OpenGLRenderer::createTextureFromPixels(int w, int h,
        const unsigned char* rgba,
        const TextureOptions& options) {
        return std::make_shared<GLTexture>(w, h, rgba, options.repeat);
    }

    std::shared_ptr<Texture> OpenGLRenderer::createTextureFromFile(const std::string& path,
        const TextureOptions& options) {
        // 1. Load the raw BMP file from disk
        SurfacePtr loaded(SDL_LoadBMP(path.c_str()));
        if (!loaded) {
            throw std::runtime_error("Cannot load BMP '" + path + "': " + SDL_GetError());
        }

        // 2. Convert into a uniform 32-bit RGBA format (8 bits per channel)
        SurfacePtr rgba(SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32));
        if (!rgba) {
            throw std::runtime_error("Cannot convert BMP '" + path + "': " + SDL_GetError());
        }

        // 3. Copy rows into a tightly packed buffer (respecting `rgba->pitch` row padding)
        const int w = rgba->w;
        const int h = rgba->h;
        const std::size_t rowBytes = static_cast<std::size_t>(w) * 4;
        std::vector<unsigned char> pixels(rowBytes * h);

        for (int y = 0; y < h; ++y) {
            const auto* srcRow = static_cast<const unsigned char*>(rgba->pixels) +
                static_cast<std::size_t>(y) * rgba->pitch;
            std::memcpy(&pixels[static_cast<std::size_t>(y) * rowBytes], srcRow, rowBytes);
        }

        // 4. Color-key transparency: Xenon 2000 BMPs use pure magenta (255, 0, 255) for transparency.
        // Zeroing RGB as well as Alpha prevents pink fringe artifacts along sprite edges.
        if (options.magentaColorKey) {
            for (std::size_t i = 0; i < pixels.size(); i += 4) {
                if (pixels[i] == 255 && pixels[i + 1] == 0 && pixels[i + 2] == 255) {
                    pixels[i] = pixels[i + 1] = pixels[i + 2] = pixels[i + 3] = 0;
                }
            }
        }

        // 5. Upload processed RGBA pixel data to an OpenGL texture object
        return std::make_shared<GLTexture>(w, h, pixels.data(), options.repeat);
    }

    void OpenGLRenderer::beginFrame(const glm::vec4& clearColor) {
        // Query actual drawable pixel dimensions (handles High-DPI / Retina displays)
        int pw = width_;
        int ph = height_;
        SDL_GetWindowSizeInPixels(window_, &pw, &ph);
        glViewport(0, 0, pw, ph);

        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind shared state once per frame rather than repeating per sprite
        shader_->use();
        shader_->setMat4("uProjection", projection_);
        shader_->setInt("uTexture", 0);
        quad_->bind();
    }

    void OpenGLRenderer::drawSprite(const SpriteDraw& sprite) {
        if (!sprite.texture) return;

        // Build Model matrix: M = Translation * Rotation * Scale
        // GLM post-multiplies matrices, so vertices scale around center, rotate around Z, then translate.
        glm::mat4 model(1.f);
        model = glm::translate(model, glm::vec3(sprite.position, 0.f));
        model = glm::rotate(model, sprite.rotation, glm::vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, glm::vec3(sprite.size * sprite.scale, 1.f));

        // Upload per-sprite uniforms and draw the unit quad
        shader_->setMat4("uModel", model);
        shader_->setVec4("uUvRect", sprite.uv);
        shader_->setVec4("uTint", sprite.tint);

        static_cast<const GLTexture*>(sprite.texture)->bind(0);
        quad_->draw();
    }

    void OpenGLRenderer::endFrame() {
        SDL_GL_SwapWindow(window_);
    }

}  // namespace xe::detail