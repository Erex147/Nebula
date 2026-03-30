#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "nebula/renderer/Texture.h"
#include "nebula/renderer/SpriteBatch.h"

namespace nebula
{
    enum class AtlasCoordinateOrigin
    {
        TopLeft,
        BottomLeft
    };

    struct TextureAtlasLoadOptions
    {
        TextureLoadOptions texture = TextureLoadOptions::pixelArt(false);
        AtlasCoordinateOrigin coordinateOrigin = AtlasCoordinateOrigin::TopLeft;
        bool insetUVs = true;

        bool operator==(const TextureAtlasLoadOptions &other) const
        {
            return texture == other.texture &&
                   coordinateOrigin == other.coordinateOrigin &&
                   insetUVs == other.insetUVs;
        }
    };

    struct AtlasRegion
    {
        glm::vec2 uvMin, uvMax; // normalised UV coords
        glm::vec2 pixelSize;    // original size in pixels
    };

    class TextureAtlas
    {
    public:
        // load a packed atlas from a .atlas descriptor + matching PNG
        // descriptor format (one line per sprite):
        //   name x y w h
        static std::shared_ptr<TextureAtlas> load(const std::string &descriptorPath,
                                                  const std::string &texturePath,
                                                  TextureAtlasLoadOptions options = TextureAtlasLoadOptions{});

        // pack individual PNGs at runtime (for small atlases)
        static std::shared_ptr<TextureAtlas> pack(
            const std::vector<std::pair<std::string, std::string>> &namePathPairs,
            int atlasSize = 1024);

        bool has(const std::string &name) const;
        AtlasRegion region(const std::string &name) const;
        const Texture &texture() const { return *m_texture; }

        void draw(SpriteBatch &batch, const std::string &name,
                  float x, float y, float w, float h,
                  const glm::vec4 &color = glm::vec4(1.0f),
                  float rotation = 0.0f);

    private:
        std::shared_ptr<Texture> m_texture;
        std::unordered_map<std::string, AtlasRegion> m_regions;
    };

} // namespace nebula
