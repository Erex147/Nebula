#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "nebula/renderer/Texture.h"
#include "nebula/renderer/Shader.h"

namespace nebula {

class AssetManager {
public:
    std::shared_ptr<Texture> loadTexture(const std::string& path);
    std::shared_ptr<Texture> texture    (const std::string& path) const;

    std::shared_ptr<Shader>  loadShader(const std::string& name,
                                        const std::string& vertPath,
                                        const std::string& fragPath);
    std::shared_ptr<Shader>  shader(const std::string& name) const;

    void clear();

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>>  m_shaders;
};

} // namespace nebula