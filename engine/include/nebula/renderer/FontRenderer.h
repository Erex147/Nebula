#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include "nebula/renderer/Texture.h"
#include "nebula/renderer/SpriteBatch.h"

namespace nebula
{

    struct Glyph
    {
        glm::vec2 uvMin, uvMax; // UV coords in the atlas
        glm::vec2 size;         // glyph size in pixels
        glm::vec2 bearing;      // offset from baseline
        float advance;          // how far to move cursor
    };

    class FontRenderer
    {
    public:
        // fontSize in pixels
        FontRenderer(const std::string &fontPath, int fontSize = 32);
        ~FontRenderer();

        void drawText(SpriteBatch &batch,
                      const std::string &text,
                      float x, float y,
                      const glm::vec4 &color = {1, 1, 1, 1},
                      float scale = 1.0f);

        float measureWidth(const std::string &text, float scale = 1.0f) const;
        float lineHeight(float scale = 1.0f) const;

        const Texture &atlasTexture() const { return *m_atlas; }

    private:
        std::unordered_map<char, Glyph> m_glyphs;
        std::unique_ptr<Texture> m_atlas;
        int m_fontSize = 32;
        float m_lineHeight = 0;
        float m_ascent = 0;

        void buildAtlas(const std::string &path, int size);
    };

} // namespace nebula
