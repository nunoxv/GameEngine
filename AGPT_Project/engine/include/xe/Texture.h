#pragma once
// Opaque GPU resources. Concrete types live in the renderer back end (src/).

namespace xe {

struct TextureOptions {
    bool repeat = false;           // GL_REPEAT wrapping (parallax layers)
    bool magentaColorKey = true;   // RGB(255,0,255) -> alpha 0 while loading
};

class Texture {
public:
    virtual ~Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    int width() const { return w_; }
    int height() const { return h_; }

protected:
    Texture(int w, int h) : w_(w), h_(h) {}

private:
    int w_, h_;
};

class Shader {
public:
    virtual ~Shader() = default;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

protected:
    Shader() = default;
};

}  // namespace xe
