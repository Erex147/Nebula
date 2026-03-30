#include "nebula/assets/AssetManager.h"
#include <stdexcept>

namespace nebula {

std::shared_ptr<Texture> AssetManager::loadTexture(const std::string& path) {
    auto [it, inserted] = m_textures.emplace(path, nullptr);
    if (inserted) it->second = std::make_shared<Texture>(path);
    return it->second;
}

std::shared_ptr<Texture> AssetManager::texture(const std::string& path) const {
    auto it = m_textures.find(path);
    if (it == m_textures.end())
        throw std::runtime_error("Texture not loaded: " + path);
    return it->second;
}

std::shared_ptr<Shader> AssetManager::loadShader(const std::string& name,
                                                   const std::string& vert,
                                                   const std::string& frag) {
    auto [it, inserted] = m_shaders.emplace(name, nullptr);
    if (inserted) it->second = std::make_shared<Shader>(vert, frag);
    return it->second;
}

std::shared_ptr<Shader> AssetManager::shader(const std::string& name) const {
    auto it = m_shaders.find(name);
    if (it == m_shaders.end())
        throw std::runtime_error("Shader not loaded: " + name);
    return it->second;
}

void AssetManager::clear() {
    m_textures.clear();
    m_shaders.clear();
}

} // namespace nebula