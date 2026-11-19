#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <xe/Renderer.h>
#include <xe/Texture.h>

namespace xe {

// Caches textures and shaders: a file is never loaded twice. Resources are RAII objects
// released when the manager (owned by the Engine) is destroyed.
class ResourceManager {
public:
    ResourceManager(Renderer& renderer, std::string assetsRoot, std::string shaderRoot);

    std::shared_ptr<Texture> texture(const std::string& relativePath,
                                     const TextureOptions& options = {});
    // Procedural texture; `fill` is only called the first time `key` is requested.
    std::shared_ptr<Texture> generatedTexture(
        const std::string& key, int width, int height,
        const std::function<void(std::vector<unsigned char>& rgba)>& fill,
        const TextureOptions& options = {});
    // Loads <shaderRoot>/<name>.vert and <name>.frag.
    std::shared_ptr<Shader> shader(const std::string& name);

    bool assetExists(const std::string& relativePath) const;
    std::size_t textureLoadCount() const { return textureLoads_; }

private:
    std::string fullPath(const std::string& relativePath) const;

    Renderer& renderer_;
    std::string assetsRoot_, shaderRoot_;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders_;
    std::size_t textureLoads_ = 0;
};

}  // namespace xe
