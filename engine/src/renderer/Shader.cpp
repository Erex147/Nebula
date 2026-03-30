#include "nebula/renderer/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace nebula {

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
    uint32_t vert = compileShader(GL_VERTEX_SHADER,   readFile(vertPath));
    uint32_t frag = compileShader(GL_FRAGMENT_SHADER, readFile(fragPath));

    m_id = glCreateProgram();
    glAttachShader(m_id, vert);
    glAttachShader(m_id, frag);
    glLinkProgram(m_id);

    int success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(m_id, 512, nullptr, log);
        throw std::runtime_error(std::string("Shader link error:\n") + log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() { glDeleteProgram(m_id); }

void Shader::bind()   const { glUseProgram(m_id); }
void Shader::unbind() const { glUseProgram(0); }

void Shader::setInt(const std::string& n, int v) {
    glUniform1i(uniformLocation(n), v);
}
void Shader::setFloat(const std::string& n, float v) {
    glUniform1f(uniformLocation(n), v);
}
void Shader::setVec2(const std::string& n, const glm::vec2& v) {
    glUniform2fv(uniformLocation(n), 1, glm::value_ptr(v));
}
void Shader::setVec4(const std::string& n, const glm::vec4& v) {
    glUniform4fv(uniformLocation(n), 1, glm::value_ptr(v));
}
void Shader::setMat4(const std::string& n, const glm::mat4& m) {
    glUniformMatrix4fv(uniformLocation(n), 1, GL_FALSE, glm::value_ptr(m));
}
void Shader::setIntArray(const std::string& n, int* values, uint32_t count) {
    glUniform1iv(uniformLocation(n), count, values);
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Could not open shader: " + path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

uint32_t Shader::compileShader(uint32_t type, const std::string& src) {
    uint32_t id = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(id, 1, &c, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(id, 512, nullptr, log);
        const char* t = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        throw std::runtime_error(std::string(t) + " compile error:\n" + log);
    }
    return id;
}

int Shader::uniformLocation(const std::string& name) {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) return it->second;
    int loc = glGetUniformLocation(m_id, name.c_str());
    if (loc == -1)
        std::cerr << "[Nebula] Uniform not found: " << name << "\n";
    return m_uniformCache[name] = loc;
}

} // namespace nebula