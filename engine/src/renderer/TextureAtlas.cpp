#include "nebula/renderer/TextureAtlas.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>

namespace nebula
{
    namespace
    {
        AtlasRegion makeRegion(float x, float y, float w, float h,
                               float textureW, float textureH,
                               TextureAtlasLoadOptions options)
        {
            const float inset = options.insetUVs ? 0.5f : 0.0f;

            AtlasRegion region;
            region.pixelSize = {w, h};

            const float left = x + inset;
            const float right = x + w - inset;
            const float top = y + inset;
            const float bottom = y + h - inset;

            if (options.coordinateOrigin == AtlasCoordinateOrigin::TopLeft)
            {
                region.uvMin = {left / textureW, 1.0f - top / textureH};
                region.uvMax = {right / textureW, 1.0f - bottom / textureH};
            }
            else
            {
                region.uvMin = {left / textureW, bottom / textureH};
                region.uvMax = {right / textureW, top / textureH};
            }
            return region;
        }
    } // namespace

    std::shared_ptr<TextureAtlas> TextureAtlas::load(
        const std::string &descPath, const std::string &texPath,
        TextureAtlasLoadOptions options)
    {
        auto atlas = std::make_shared<TextureAtlas>();
        atlas->m_texture = std::make_shared<Texture>(texPath, options.texture);

        float tw = (float)atlas->m_texture->width();
        float th = (float)atlas->m_texture->height();

        std::ifstream file(descPath);
        if (!file.is_open())
            throw std::runtime_error("Cannot open atlas descriptor: " + descPath);

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;
            std::istringstream ss(line);
            std::string name;
            float x, y, w, h;
            ss >> name >> x >> y >> w >> h;

            atlas->m_regions[name] = makeRegion(x, y, w, h, tw, th, options);
        }
        return atlas;
    }

    // simple rectangle packer
    struct PackRect
    {
        std::string name;
        int x = 0, y = 0, w = 0, h = 0;
        std::vector<unsigned char> pixels;
        int channels = 0;
    };

    std::shared_ptr<TextureAtlas> TextureAtlas::pack(
        const std::vector<std::pair<std::string, std::string>> &pairs, int atlasSize)
    {
        std::vector<PackRect> rects;
        rects.reserve(pairs.size());

        for (auto &[name, path] : pairs)
        {
            PackRect r;
            r.name = name;
            stbi_set_flip_vertically_on_load(true);
            unsigned char *data = stbi_load(path.c_str(), &r.w, &r.h, &r.channels, 4);
            if (!data)
                throw std::runtime_error("Failed to load: " + path);
            r.channels = 4;
            r.pixels.assign(data, data + r.w * r.h * 4);
            stbi_image_free(data);
            rects.push_back(std::move(r));
        }

        // sort largest first for better packing
        std::sort(rects.begin(), rects.end(),
                  [](const PackRect &a, const PackRect &b)
                  {
                      return (a.w * a.h) > (b.w * b.h);
                  });

        // simple shelf packer
        std::vector<unsigned char> buffer(atlasSize * atlasSize * 4, 0);
        int shelfX = 0, shelfY = 0, shelfH = 0;

        auto atlas = std::make_shared<TextureAtlas>();
        for (auto &r : rects)
        {
            if (shelfX + r.w > atlasSize)
            {
                shelfX = 0;
                shelfY += shelfH + 1;
                shelfH = 0;
            }
            r.x = shelfX;
            r.y = shelfY;

            // blit into buffer
            for (int row = 0; row < r.h; row++)
            {
                std::memcpy(
                    buffer.data() + ((shelfY + row) * atlasSize + shelfX) * 4,
                    r.pixels.data() + row * r.w * 4,
                    r.w * 4);
            }

            TextureAtlasLoadOptions options;
            options.texture = TextureLoadOptions::pixelArt(false);
            options.coordinateOrigin = AtlasCoordinateOrigin::BottomLeft;
            options.insetUVs = false;
            atlas->m_regions[r.name] = makeRegion((float)shelfX, (float)shelfY,
                                                  (float)r.w, (float)r.h,
                                                  (float)atlasSize, (float)atlasSize,
                                                  options);

            shelfX += r.w + 1;
            shelfH = std::max(shelfH, r.h);
        }

        // upload
        uint32_t texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasSize, atlasSize,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // wrap in a Texture object
        // we need a way to adopt an existing GL id — add this constructor to Texture
        atlas->m_texture = std::make_shared<Texture>(atlasSize, atlasSize);
        // re-upload using the atlas texture's id
        glBindTexture(GL_TEXTURE_2D, atlas->m_texture->id());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasSize, atlasSize,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &texID);

        return atlas;
    }

    bool TextureAtlas::has(const std::string &name) const
    {
        return m_regions.count(name) > 0;
    }

    AtlasRegion TextureAtlas::region(const std::string &name) const
    {
        auto it = m_regions.find(name);
        if (it == m_regions.end())
            throw std::runtime_error("Atlas region not found: " + name);
        return it->second;
    }

    void TextureAtlas::draw(SpriteBatch &batch, const std::string &name,
                            float x, float y, float w, float h,
                            const glm::vec4 &color, float rotation)
    {
        auto r = region(name);
        batch.drawRegion(*m_texture, x, y, w, h,
                         r.uvMin.x, r.uvMin.y,
                         r.uvMax.x, r.uvMax.y,
                         color);
    }

} // namespace nebula
