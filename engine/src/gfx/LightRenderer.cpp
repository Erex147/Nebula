#include "nebula/gfx/LightRenderer.h"
#include <glad/glad.h>

namespace nebula
{

    LightRenderer::LightRenderer(int w, int h, Shader &shader)
    {
        FramebufferSpec spec;
        spec.width = w;
        spec.height = h;
        spec.hdr = true;
        m_lightFBO = std::make_unique<Framebuffer>(spec);
        m_batch = std::make_unique<SpriteBatch>(shader);
    }

    void LightRenderer::begin(const glm::mat4 &vp)
    {
        m_lightFBO->bind();

        // clear to ambient colour
        glClearColor(ambientColor.r * ambientStrength,
                     ambientColor.g * ambientStrength,
                     ambientColor.b * ambientStrength, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // additive blending — lights stack on top of each other
        glBlendFunc(GL_ONE, GL_ONE);

        m_batch->begin(vp);
    }

    void LightRenderer::submitLight(const PointLight &l)
    {
        glm::vec4 color{l.color.r, l.color.g, l.color.b, l.intensity};
        float r = l.radius;
        m_batch->drawColorQuad(
            l.position.x - r, l.position.y - r,
            r * 2.0f, r * 2.0f, color);
    }

    void LightRenderer::end()
    {
        m_batch->flush();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // restore
        m_lightFBO->unbind();
    }

    uint32_t LightRenderer::lightMapID() const
    {
        return m_lightFBO->colorAttachmentID();
    }

    void LightRenderer::resize(int w, int h)
    {
        m_lightFBO->resize(w, h);
    }

} // namespace nebula