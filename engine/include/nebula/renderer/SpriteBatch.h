#pragma once
#include <array>
#include <glm/glm.hpp>
#include "nebula/renderer/Texture.h"
#include "nebula/renderer/Shader.h"
#include "nebula/renderer/VertexArray.h"

namespace nebula
{

    struct QuadVertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 color;
        float texIndex;
    };

    class SpriteBatch
    {
    public:
        static constexpr uint32_t MAX_QUADS = 10000;
        static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
        static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;
        static constexpr uint32_t MAX_TEXTURES = 16;

        explicit SpriteBatch(Shader &shader);

        // Call once per frame before any draw()
        void begin(const glm::mat4 &viewProjection);

        // Queue a sprite — flushes automatically if buffers are full
        void draw(const Texture &texture,
                  float x, float y, float w, float h,
                  const glm::vec4 &color = glm::vec4(1.0f),
                  float rotationRad = 0.0f);

        // draw a specific UV region of a texture (used by tilemap, sprite sheets)
        void drawRegion(const Texture &texture,
                        float x, float y, float w, float h,
                        float u0, float v0, float u1, float v1,
                        const glm::vec4 &color = glm::vec4(1.0f));

        // draw a solid colored quad — no texture lookup
        void drawColorQuad(float x, float y, float w, float h,
                           const glm::vec4 &color);

        // Send everything queued this frame to the GPU
        void flush();

        uint32_t drawCalls() const { return m_drawCalls; }
        uint32_t quadCount() const { return m_quadCount; }

    private:
        Shader &m_shader;
        VertexArray m_vao;
        uint32_t m_whiteTexID = 0;

        std::array<QuadVertex, MAX_VERTICES> m_vertices;
        std::array<uint32_t, MAX_TEXTURES> m_textures = {};
        uint32_t m_quadCount = 0;
        uint32_t m_texCount = 0;
        uint32_t m_drawCalls = 0;

        void flushInternal();
        float getOrAddTexture(uint32_t id);
        void buildIndices();
    };

} // namespace nebula