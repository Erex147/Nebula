#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace nebula {

class Shader {
public:
    Shader(const std::string& vertPath, const std::string& fragPath);
    ~Shader();

    void bind()   const;
    void unbind() const;

    void setInt     (const std::string& name, int value);
    void setFloat   (const std::string& name, float value);
    void setVec2    (const std::string& name, const glm::vec2& v);
    void setVec4    (const std::string& name, const glm::vec4& v);
    void setMat4    (const std::string& name, const glm::mat4& m);
    void setIntArray(const std::string& name, int* values, uint32_t count);

    uint32_t id() const { return m_id; }

private:
    uint32_t m_id = 0;
    std::unordered_map<std::string, int> m_uniformCache;

    std::string readFile(const std::string& path);
    uint32_t    compileShader(uint32_t type, const std::string& src);
    int         uniformLocation(const std::string& name);
};

} // namespace nebula