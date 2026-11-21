#pragma once

// PRIVATE header: OpenGL 3.3 Core implementation of the abstract xe::Renderer interface.
// Never included by game code (hidden behind Engine's Pimpl).
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

            // Loads the base sprite shader and creates the unit quad VAO/VBO (requires active GL context)
            void initialize(ResourceManager& resources);

            // Resource factory methods called by ResourceManager
            std::shared_ptr<Shader> createShader(const std::string& vertexPath,
                const std::string& fragmentPath) override;
            std::shared_ptr<Texture> createTextureFromFile(const std::string& path,
                const TextureOptions& options) override;
            std::shared_ptr<Texture> createTextureFromPixels(int width, int height,
                const unsigned char* rgba,
                const TextureOptions& options) override;

            // Per-frame rendering pipeline
            void beginFrame(const glm::vec4& clearColor) override;
            void drawSprite(const SpriteDraw& sprite) override;
            void endFrame() override;

            glm::vec2 viewportSize() const override { return { float(width_), float(height_) }; }

        private:
            SDL_Window* window_;               // Non-owning handle (owned by xe::Window)
            int width_, height_;               // Logical game resolution in pixels
            std::shared_ptr<GLShader> shader_; // Cached sprite shader program
            std::unique_ptr<QuadMesh> quad_;   // Shared unit quad VAO/VBO reused for every sprite
            glm::mat4 projection_;             // Orthographic projection matrix (pixel coordinates)
        };

    }  // namespace detail
}  // namespace xe