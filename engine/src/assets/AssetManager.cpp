#include "nebula/assets/AssetManager.h"
#include <stdexcept>

namespace nebula {
namespace
{
size_t hashCombine(size_t seed, size_t value)
{
    return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

size_t hashTextureOptions(const TextureLoadOptions& options)
{
    size_t seed = 0;
    seed = hashCombine(seed, std::hash<bool>{}(options.flipVertically));
    seed = hashCombine(seed, std::hash<bool>{}(options.generateMipmaps));
    seed = hashCombine(seed, std::hash<int>{}(static_cast<int>(options.minFilter)));
    seed = hashCombine(seed, std::hash<int>{}(static_cast<int>(options.magFilter)));
    seed = hashCombine(seed, std::hash<int>{}(static_cast<int>(options.wrapS)));
    seed = hashCombine(seed, std::hash<int>{}(static_cast<int>(options.wrapT)));
    return seed;
}

size_t hashAtlasOptions(const TextureAtlasLoadOptions& options)
{
    size_t seed = hashTextureOptions(options.texture);
    seed = hashCombine(seed, std::hash<int>{}(static_cast<int>(options.coordinateOrigin)));
    seed = hashCombine(seed, std::hash<bool>{}(options.insetUVs));
    return seed;
}
} // namespace

size_t AssetManager::TextureKeyHash::operator()(const TextureKey& key) const
{
    size_t seed = std::hash<std::string>{}(key.path);
    return hashCombine(seed, hashTextureOptions(key.options));
}

size_t AssetManager::TextureAtlasKeyHash::operator()(const TextureAtlasKey& key) const
{
    size_t seed = std::hash<std::string>{}(key.descriptorPath);
    seed = hashCombine(seed, std::hash<std::string>{}(key.texturePath));
    return hashCombine(seed, hashAtlasOptions(key.options));
}

size_t AssetManager::FontKeyHash::operator()(const FontKey& key) const
{
    size_t seed = std::hash<std::string>{}(key.path);
    return hashCombine(seed, std::hash<int>{}(key.fontSize));
}

std::shared_ptr<Texture> AssetManager::loadTexture(const std::string& path, TextureLoadOptions options) {
    TextureKey key{path, options};
    auto [it, inserted] = m_textures.emplace(key, nullptr);
    if (inserted) it->second = std::make_shared<Texture>(path, options);
    return it->second;
}

std::shared_ptr<Texture> AssetManager::texture(const std::string& path, TextureLoadOptions options) const {
    auto it = m_textures.find(TextureKey{path, options});
    if (it == m_textures.end())
        throw std::runtime_error("Texture not loaded: " + path);
    return it->second;
}

std::shared_ptr<TextureAtlas> AssetManager::loadAtlas(const std::string& descriptorPath,
                                                      const std::string& texturePath,
                                                      TextureAtlasLoadOptions options) {
    TextureAtlasKey key{descriptorPath, texturePath, options};
    auto [it, inserted] = m_atlases.emplace(key, nullptr);
    if (inserted)
        it->second = TextureAtlas::load(descriptorPath, texturePath, options);
    return it->second;
}

std::shared_ptr<TextureAtlas> AssetManager::atlas(const std::string& descriptorPath,
                                                  const std::string& texturePath,
                                                  TextureAtlasLoadOptions options) const {
    auto it = m_atlases.find(TextureAtlasKey{descriptorPath, texturePath, options});
    if (it == m_atlases.end())
        throw std::runtime_error("Atlas not loaded: " + descriptorPath);
    return it->second;
}

std::shared_ptr<FontRenderer> AssetManager::loadFont(const std::string& path, int fontSize)
{
    FontKey key{path, fontSize};
    auto [it, inserted] = m_fonts.emplace(key, nullptr);
    if (inserted)
        it->second = std::make_shared<FontRenderer>(path, fontSize);
    return it->second;
}

std::shared_ptr<FontRenderer> AssetManager::font(const std::string& path, int fontSize) const
{
    auto it = m_fonts.find(FontKey{path, fontSize});
    if (it == m_fonts.end())
        throw std::runtime_error("Font not loaded: " + path);
    return it->second;
}

std::shared_ptr<AudioClip> AssetManager::loadAudioClip(const std::string& name, const std::string& path)
{
    auto [it, inserted] = m_audioClips.emplace(name, nullptr);
    if (inserted)
        it->second = std::make_shared<AudioClip>(AudioClip{name, path});
    else
        it->second->path = path;
    return it->second;
}

std::shared_ptr<AudioClip> AssetManager::audioClip(const std::string& name) const
{
    auto it = m_audioClips.find(name);
    if (it == m_audioClips.end())
        throw std::runtime_error("Audio clip not loaded: " + name);
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
    m_atlases.clear();
    m_fonts.clear();
    m_audioClips.clear();
    m_shaders.clear();
}

} // namespace nebula
