#include <xe/ResourceManager.h>

#include <filesystem>

namespace xe {

ResourceManager::ResourceManager(Renderer& renderer, std::string assetsRoot, std::string shaderRoot)
    : renderer_(renderer), assetsRoot_(std::move(assetsRoot)), shaderRoot_(std::move(shaderRoot)) {}

std::string ResourceManager::fullPath(const std::string& rel) const {
    return (std::filesystem::path(assetsRoot_) / rel).string();
}

bool ResourceManager::assetExists(const std::string& rel) const {
    return std::filesystem::exists(fullPath(rel));
}

std::shared_ptr<Texture> ResourceManager::texture(const std::string& rel, const TextureOptions& o) {
    auto it = textures_.find(rel);
    if (it != textures_.end()) return it->second;   // cached: never load a file twice
    auto tex = renderer_.createTextureFromFile(fullPath(rel), o);
    textures_.emplace(rel, tex);
    ++textureLoads_;
    return tex;
}

std::shared_ptr<Texture> ResourceManager::generatedTexture(
    const std::string& key, int w, int h,
    const std::function<void(std::vector<unsigned char>&)>& fill, const TextureOptions& o) {
    const std::string cacheKey = "generated:" + key;
    auto it = textures_.find(cacheKey);
    if (it != textures_.end()) return it->second;
    std::vector<unsigned char> rgba(static_cast<size_t>(w) * h * 4, 0);
    fill(rgba);
    auto tex = renderer_.createTextureFromPixels(w, h, rgba.data(), o);
    textures_.emplace(cacheKey, tex);
    ++textureLoads_;
    return tex;
}

std::shared_ptr<Shader> ResourceManager::shader(const std::string& name) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) return it->second;
    const auto base = std::filesystem::path(shaderRoot_) / name;
    auto sh = renderer_.createShader(base.string() + ".vert", base.string() + ".frag");
    shaders_.emplace(name, sh);
    return sh;
}

}  // namespace xe
