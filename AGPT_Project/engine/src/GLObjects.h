#pragma once
// PRIVATE header: RAII wrappers for OpenGL objects (texture, shader program, quad mesh).
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <xe/Texture.h>

namespace xe::detail {

class GLTexture final : public Texture {
public:
    GLTexture(int w, int h, const unsigned char* rgba, bool repeat) : Texture(w, h) {
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // crisp pixel art
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        const GLint wrap = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    }
    ~GLTexture() override { glDeleteTextures(1, &id_); }

    void bind(unsigned unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id_);
    }

private:
    GLuint id_ = 0;
};

class GLShader final : public Shader {
public:
    GLShader(const std::string& vertexPath, const std::string& fragmentPath) {
        Stage vs(GL_VERTEX_SHADER, readFile(vertexPath), vertexPath);
        Stage fs(GL_FRAGMENT_SHADER, readFile(fragmentPath), fragmentPath);
        program_ = glCreateProgram();
        glAttachShader(program_, vs.id);
        glAttachShader(program_, fs.id);
        glLinkProgram(program_);
        GLint ok = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &ok);
        if (!ok) {
            std::string log = programLog(program_);
            glDeleteProgram(program_);
            throw std::runtime_error("Shader link error (" + vertexPath + " + " + fragmentPath +
                                     "):\n" + log);
        }
        glDetachShader(program_, vs.id);
        glDetachShader(program_, fs.id);
    }
    ~GLShader() override { glDeleteProgram(program_); }

    void use() const { glUseProgram(program_); }
    void setInt(const char* n, int v) { glUniform1i(loc(n), v); }
    void setVec4(const char* n, const glm::vec4& v) { glUniform4fv(loc(n), 1, glm::value_ptr(v)); }
    void setMat4(const char* n, const glm::mat4& m) {
        glUniformMatrix4fv(loc(n), 1, GL_FALSE, glm::value_ptr(m));
    }

private:
    // One compiled stage; the destructor deletes it even when linking later throws.
    struct Stage {
        GLuint id = 0;
        Stage(GLenum type, const std::string& source, const std::string& name) {
            id = glCreateShader(type);
            const char* src = source.c_str();
            glShaderSource(id, 1, &src, nullptr);
            glCompileShader(id);
            GLint ok = GL_FALSE;
            glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                GLint len = 0;
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
                std::string log(static_cast<size_t>(len > 1 ? len : 1), '\0');
                glGetShaderInfoLog(id, len, nullptr, log.data());
                glDeleteShader(id);
                throw std::runtime_error("Shader compile error (" + name + "):\n" + log);
            }
        }
        ~Stage() { glDeleteShader(id); }
        Stage(const Stage&) = delete;
        Stage& operator=(const Stage&) = delete;
    };

    static std::string readFile(const std::string& path) {
        std::ifstream f(path);
        if (!f) throw std::runtime_error("Cannot open shader file: " + path);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
    static std::string programLog(GLuint p) {
        GLint len = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetProgramInfoLog(p, len, nullptr, log.data());
        return log;
    }
    GLint loc(const char* name) {
        auto it = locations_.find(name);
        if (it != locations_.end()) return it->second;
        GLint l = glGetUniformLocation(program_, name);
        locations_.emplace(name, l);
        return l;
    }

    GLuint program_ = 0;
    std::unordered_map<std::string, GLint> locations_;
};

// Unit quad centred on the origin: VAO + VBO + EBO.
class QuadMesh {
public:
    QuadMesh() {
        const float vertices[] = {
            // x     y     u    v
            -0.5f, -0.5f, 0.f, 0.f,   // top-left
             0.5f, -0.5f, 1.f, 0.f,   // top-right
             0.5f,  0.5f, 1.f, 1.f,   // bottom-right
            -0.5f,  0.5f, 0.f, 1.f    // bottom-left
        };
        const unsigned indices[] = {0, 1, 2, 2, 3, 0};
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }
    ~QuadMesh() {
        glDeleteBuffers(1, &ebo_);
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }
    QuadMesh(const QuadMesh&) = delete;
    QuadMesh& operator=(const QuadMesh&) = delete;

    void bind() const { glBindVertexArray(vao_); }
    void draw() const { glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); }

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
};

}  // namespace xe::detail
