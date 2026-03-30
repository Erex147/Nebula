#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "nebula/audio/AudioClip.h"
#include "nebula/renderer/FontRenderer.h"
#include "nebula/renderer/Texture.h"
#include "nebula/renderer/Shader.h"
#include "nebula/renderer/TextureAtlas.h"

namespace nebula {

class AssetManager {
public:
    std::shared_ptr<Texture> loadTexture(const std::string& path,
                                         TextureLoadOptions options = TextureLoadOptions{});
    std::shared_ptr<Texture> texture(const std::string& path,
                                     TextureLoadOptions options = TextureLoadOptions{}) const;
    std::shared_ptr<TextureAtlas> loadAtlas(const std::string& descriptorPath,
                                            const std::string& texturePath,
                                            TextureAtlasLoadOptions options = TextureAtlasLoadOptions{});
    std::shared_ptr<TextureAtlas> atlas(const std::string& descriptorPath,
                                        const std::string& texturePath,
                                        TextureAtlasLoadOptions options = TextureAtlasLoadOptions{}) const;
    std::shared_ptr<FontRenderer> loadFont(const std::string& path, int fontSize = 32);
    std::shared_ptr<FontRenderer> font(const std::string& path, int fontSize = 32) const;
    std::shared_ptr<AudioClip> loadAudioClip(const std::string& name, const std::string& path);
    std::shared_ptr<AudioClip> audioClip(const std::string& name) const;

    std::shared_ptr<Shader>  loadShader(const std::string& name,
                                        const std::string& vertPath,
                                        const std::string& fragPath);
    std::shared_ptr<Shader>  shader(const std::string& name) const;

    void clear();

private:
    struct TextureKey
    {
        std::string path;
        TextureLoadOptions options;

        bool operator==(const TextureKey& other) const = default;
    };

    struct TextureAtlasKey
    {
        std::string descriptorPath;
        std::string texturePath;
        TextureAtlasLoadOptions options;

        bool operator==(const TextureAtlasKey& other) const = default;
    };

    struct TextureKeyHash
    {
        size_t operator()(const TextureKey& key) const;
    };

    struct TextureAtlasKeyHash
    {
        size_t operator()(const TextureAtlasKey& key) const;
    };

    struct FontKey
    {
        std::string path;
        int fontSize = 32;

        bool operator==(const FontKey& other) const = default;
    };

    struct FontKeyHash
    {
        size_t operator()(const FontKey& key) const;
    };

    std::unordered_map<TextureKey, std::shared_ptr<Texture>, TextureKeyHash> m_textures;
    std::unordered_map<TextureAtlasKey, std::shared_ptr<TextureAtlas>, TextureAtlasKeyHash> m_atlases;
    std::unordered_map<FontKey, std::shared_ptr<FontRenderer>, FontKeyHash> m_fonts;
    std::unordered_map<std::string, std::shared_ptr<AudioClip>> m_audioClips;
    std::unordered_map<std::string, std::shared_ptr<Shader>>  m_shaders;
};

} // namespace nebula
