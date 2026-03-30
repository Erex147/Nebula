#include "nebula/renderer/SpriteBatch.h"
#include <glad/glad.h>
#include <cmath>

namespace nebula
{

    SpriteBatch::SpriteBatch(Shader &shader) : m_shader(shader)
    {
        buildIndices();

        m_vao.setVertexData(nullptr, MAX_VERTICES * sizeof(QuadVertex), true);
        m_vao.setLayout({
            {GL_FLOAT, 2, false}, // position
            {GL_FLOAT, 2, false}, // texCoord
            {GL_FLOAT, 4, false}, // color
            {GL_FLOAT, 1, false}, // texIndex
        });

        // 1x1 white texture — used for solid-color draws
        unsigned char white[4] = {255, 255, 255, 255};
        glGenTextures(1, &m_whiteTexID);
        glBindTexture(GL_TEXTURE_2D, m_whiteTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Pre-bind sampler slots
        m_shader.bind();
        int samplers[MAX_TEXTURES];
        for (int i = 0; i < (int)MAX_TEXTURES; i++)
            samplers[i] = i;
        m_shader.setIntArray("u_Textures", samplers, MAX_TEXTURES);
        m_shader.unbind();
    }

    void SpriteBatch::begin(const glm::mat4 &vp)
    {
        m_quadCount = 0;
        m_texCount = 0;
        m_drawCalls = 0;
        m_shader.bind();
        m_shader.setMat4("u_ViewProjection", vp);
    }

    void SpriteBatch::draw(const Texture &tex,
                           float x, float y, float w, float h,
                           const glm::vec4 &color, float rot)
    {
        if (m_quadCount >= MAX_QUADS || m_texCount >= MAX_TEXTURES)
            flushInternal();

        float ti = getOrAddTexture(tex.id());

        glm::vec2 p[4] = {
            {x, y},
            {x + w, y},
            {x + w, y + h},
            {x, y + h},
        };

        if (rot != 0.0f)
        {
            float cx = x + w * 0.5f, cy = y + h * 0.5f;
            float c = cosf(rot), s = sinf(rot);
            for (auto &v : p)
            {
                float rx = v.x - cx, ry = v.y - cy;
                v.x = cx + rx * c - ry * s;
                v.y = cy + rx * s + ry * c;
            }
        }

        glm::vec2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        uint32_t base = m_quadCount * 4;
        for (int i = 0; i < 4; i++)
            m_vertices[base + i] = {p[i], uv[i], color, ti};

        m_quadCount++;
    }

    void SpriteBatch::drawRegion(const Texture &tex,
                                 float x, float y, float w, float h,
                                 float u0, float v0, float u1, float v1,
                                 const glm::vec4 &color)
    {
        if (m_quadCount >= MAX_QUADS || m_texCount >= MAX_TEXTURES)
            flushInternal();

        float ti = getOrAddTexture(tex.id());
        uint32_t base = m_quadCount * 4;

        m_vertices[base + 0] = {{x, y}, {u0, v0}, color, ti};
        m_vertices[base + 1] = {{x + w, y}, {u1, v0}, color, ti};
        m_vertices[base + 2] = {{x + w, y + h}, {u1, v1}, color, ti};
        m_vertices[base + 3] = {{x, y + h}, {u0, v1}, color, ti};
        m_quadCount++;
    }

    void SpriteBatch::drawColorQuad(float x, float y, float w, float h,
                                    const glm::vec4 &color)
    {
        if (m_quadCount >= MAX_QUADS)
            flushInternal();

        // slot 0 is always the white texture
        float ti = getOrAddTexture(m_whiteTexID);
        uint32_t base = m_quadCount * 4;

        glm::vec2 p[4] = {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
        glm::vec2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        for (int i = 0; i < 4; i++)
            m_vertices[base + i] = {p[i], uv[i], color, ti};
        m_quadCount++;
    }

    void SpriteBatch::flush()
    {
        flushInternal();
        m_shader.unbind();
    }

    void SpriteBatch::flushInternal()
    {
        if (m_quadCount == 0)
            return;

        for (uint32_t i = 0; i < m_texCount; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        }

        m_vao.setVertexData(m_vertices.data(),
                            m_quadCount * 4 * sizeof(QuadVertex), true);
        m_vao.bind();
        glDrawElements(GL_TRIANGLES, m_quadCount * 6, GL_UNSIGNED_INT, nullptr);
        m_vao.unbind();

        m_drawCalls++;
        m_quadCount = 0;
        m_texCount = 0;
    }

    float SpriteBatch::getOrAddTexture(uint32_t id)
    {
        for (uint32_t i = 0; i < m_texCount; i++)
            if (m_textures[i] == id)
                return (float)i;
        m_textures[m_texCount] = id;
        return (float)m_texCount++;
    }

    void SpriteBatch::buildIndices()
    {
        std::array<uint32_t, MAX_INDICES> idx;
        for (uint32_t i = 0, v = 0; i < MAX_INDICES; i += 6, v += 4)
        {
            idx[i + 0] = v + 0;
            idx[i + 1] = v + 1;
            idx[i + 2] = v + 2;
            idx[i + 3] = v + 2;
            idx[i + 4] = v + 3;
            idx[i + 5] = v + 0;
        }
        m_vao.setIndexData(idx.data(), MAX_INDICES);
    }

} // namespace nebula