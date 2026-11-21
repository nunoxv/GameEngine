#include <xe/ResourceManager.h>

#include <filesystem>
#include <utility>

namespace xe {

    ResourceManager::ResourceManager(Renderer& renderer, std::string assetsRoot, std::string shaderRoot)
        : renderer_(renderer),
        assetsRoot_(std::move(assetsRoot)),
        shaderRoot_(std::move(shaderRoot)) {
    }

    std::string ResourceManager::fullPath(const std::string& relativePath) const {
        return (std::filesystem::path(assetsRoot_) / relativePath).string();
    }

    bool ResourceManager::assetExists(const std::string& relativePath) const {
        return std::filesystem::exists(fullPath(relativePath));
    }

    std::shared_ptr<Texture> ResourceManager::texture(const std::string& relativePath,
        const TextureOptions& options) {
        // 1. Cache hit: return existing GPU texture immediately without disk I/O
        if (auto it = textures_.find(relativePath); it != textures_.end()) {
            return it->second;
        }

        // 2. Cache miss: load via the abstract Renderer backend, store, and return
        auto tex = renderer_.createTextureFromFile(fullPath(relativePath), options);
        textures_.emplace(relativePath, tex);
        ++textureLoads_;
        return tex;
    }

    std::shared_ptr<Texture> ResourceManager::generatedTexture(
        const std::string& key, int width, int height,
        const std::function<void(std::vector<unsigned char>&)>& fill,
        const TextureOptions& options) {
        // Prefix key so procedural textures never collide with file paths
        const std::string cacheKey = "generated:" + key;
        if (auto it = textures_.find(cacheKey); it != textures_.end()) {
            return it->second;
        }

        // Allocate zero-initialized RGBA buffer, let caller populate pixels, and upload to GPU
        std::vector<unsigned char> rgba(static_cast<size_t>(width) * height * 4, 0);
        fill(rgba);

        auto tex = renderer_.createTextureFromPixels(width, height, rgba.data(), options);
        textures_.emplace(cacheKey, tex);
        ++textureLoads_;
        return tex;
    }

    std::shared_ptr<Shader> ResourceManager::shader(const std::string& name) {
        if (auto it = shaders_.find(name); it != shaders_.end()) {
            return it->second;
        }

        const auto base = std::filesystem::path(shaderRoot_) / name;
        auto compiled = renderer_.createShader(base.string() + ".vert", base.string() + ".frag");
        shaders_.emplace(name, compiled);
        return compiled;
    }

}  // namespace xe