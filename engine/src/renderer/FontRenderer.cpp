#include "nebula/renderer/FontRenderer.h"
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdexcept>
#include <iostream>
#include <cmath>

namespace nebula
{

    // atlas is 512x512, glyphs packed left to right
    static constexpr int ATLAS_W = 512;
    static constexpr int ATLAS_H = 512;
    static constexpr int GLYPH_PADDING = 2;

    FontRenderer::FontRenderer(const std::string &path, int fontSize)
        : m_fontSize(fontSize)
    {
        buildAtlas(path, fontSize);
    }

    FontRenderer::~FontRenderer() {}

    void FontRenderer::buildAtlas(const std::string &path, int size)
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
            throw std::runtime_error("Failed to init FreeType");

        FT_Face face;
        if (FT_New_Face(ft, path.c_str(), 0, &face))
            throw std::runtime_error("Failed to load font: " + path);

        FT_Set_Pixel_Sizes(face, 0, size);
        m_lineHeight = (float)(face->size->metrics.height >> 6);
        m_ascent = (float)(face->size->metrics.ascender >> 6);

        // allocate a blank atlas
        std::vector<unsigned char> buffer(ATLAS_W * ATLAS_H, 0);

        int curX = GLYPH_PADDING, curY = GLYPH_PADDING, rowH = 0;

        for (unsigned char c = 32; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                continue;
            FT_GlyphSlot g = face->glyph;

            if (curX + (int)g->bitmap.width + GLYPH_PADDING >= ATLAS_W)
            {
                curX = GLYPH_PADDING;
                curY += rowH + GLYPH_PADDING;
                rowH = 0;
            }

            for (unsigned int row = 0; row < g->bitmap.rows; row++)
            {
                for (unsigned int col = 0; col < g->bitmap.width; col++)
                {
                    buffer[(curY + row) * ATLAS_W + (curX + col)] =
                        g->bitmap.buffer[row * g->bitmap.pitch + col];
                }
            }

            Glyph glyph;
            // Pull UVs half a texel inward to avoid filtering neighboring glyphs.
            glyph.uvMin = {((float)curX + 0.5f) / ATLAS_W,
                           ((float)curY + 0.5f) / ATLAS_H};
            glyph.uvMax = {((float)(curX + g->bitmap.width) - 0.5f) / ATLAS_W,
                           ((float)(curY + g->bitmap.rows) - 0.5f) / ATLAS_H};
            glyph.size = {(float)g->bitmap.width, (float)g->bitmap.rows};
            glyph.bearing = {(float)g->bitmap_left, (float)g->bitmap_top};
            glyph.advance = (float)(g->advance.x >> 6);
            m_glyphs[c] = glyph;

            rowH = std::max(rowH, (int)g->bitmap.rows);
            curX += g->bitmap.width + GLYPH_PADDING;
        }

        // upload as single-channel R8 texture
        m_atlas = std::make_unique<Texture>(ATLAS_W, ATLAS_H);
        glBindTexture(GL_TEXTURE_2D, m_atlas->id());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_W, ATLAS_H, 0,
                     GL_RED, GL_UNSIGNED_BYTE, buffer.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void FontRenderer::drawText(SpriteBatch &batch, const std::string &text,
                                float x, float y, const glm::vec4 &color, float scale)
    {
        float cursorX = x;
        const float baselineY = y + m_ascent * scale;

        for (char c : text)
        {
            auto it = m_glyphs.find(c);
            if (it == m_glyphs.end())
                continue;
            const Glyph &g = it->second;

            if (g.size.x == 0 || g.size.y == 0)
            {
                cursorX += g.advance * scale;
                continue;
            }

            float px = std::round(cursorX + g.bearing.x * scale);
            float py = std::round(baselineY - g.bearing.y * scale);

            batch.drawRegion(*m_atlas,
                             px, py,
                             g.size.x * scale, g.size.y * scale,
                             g.uvMin.x, g.uvMin.y,
                             g.uvMax.x, g.uvMax.y,
                             color);

            cursorX += g.advance * scale;
        }
    }

    float FontRenderer::measureWidth(const std::string &text, float scale) const
    {
        float w = 0;
        for (char c : text)
        {
            auto it = m_glyphs.find(c);
            if (it != m_glyphs.end())
                w += it->second.advance * scale;
        }
        return w;
    }

    float FontRenderer::lineHeight(float scale) const
    {
        return m_lineHeight * scale;
    }

} // namespace nebula
