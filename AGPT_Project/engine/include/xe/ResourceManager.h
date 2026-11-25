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

    // Central asset cache for Textures and Shaders.
    // Guarantees that no file is loaded from disk or uploaded to the GPU more than once.
    // All cached resources are RAII objects freed automatically when Engine shuts down.
    class ResourceManager {
    public:
        ResourceManager(Renderer& renderer, std::string assetsRoot, std::string shaderRoot);

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        // Loads and caches a texture relative to `assetsRoot` (returns cached copy on repeat calls)
        std::shared_ptr<Texture> texture(const std::string& relativePath,
            const TextureOptions& options = {});

        // Builds and caches a procedural RGBA texture in memory; `fill` runs only on the first call
        std::shared_ptr<Texture> generatedTexture(
            const std::string& key, int width, int height,
            const std::function<void(std::vector<unsigned char>& rgba)>& fill,
            const TextureOptions& options = {});

        // Loads and caches <shaderRoot>/<name>.vert and <shaderRoot>/<name>.frag
        std::shared_ptr<Shader> shader(const std::string& name);

        bool assetExists(const std::string& relativePath) const;

    private:
        std::string fullPath(const std::string& relativePath) const;

        Renderer& renderer_;
        std::string assetsRoot_;
        std::string shaderRoot_;
        std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
        std::unordered_map<std::string, std::shared_ptr<Shader>> shaders_;
    };

}  // namespace xe