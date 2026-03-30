#include "nebula/gfx/PostProcessor.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace nebula
{

    static float s_quadVerts[] = {
        // pos       // uv
        -1,
        1,
        0,
        1,
        -1,
        -1,
        0,
        0,
        1,
        -1,
        1,
        0,
        -1,
        1,
        0,
        1,
        1,
        -1,
        1,
        0,
        1,
        1,
        1,
        1,
    };

    PostProcessor::PostProcessor(int w, int h, Shader &shader)
        : m_shader(shader)
    {
        FramebufferSpec spec;
        spec.width = w;
        spec.height = h;
        spec.hdr = true;
        m_sceneFBO = std::make_unique<Framebuffer>(spec);
        initQuad();
    }

    void PostProcessor::initQuad()
    {
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_quadVerts),
                     s_quadVerts, GL_STATIC_DRAW);
        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), (void *)0);
        // uv
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), (void *)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    void PostProcessor::beginScene()
    {
        m_sceneFBO->bind();
        m_sceneFBO->clear();
    }

    void PostProcessor::endScene()
    {
        m_sceneFBO->unbind();
    }

    void PostProcessor::composite(uint32_t lightMapID, const PostFX &fx)
    {
        int backbufferW = width();
        int backbufferH = height();
        if (GLFWwindow *window = glfwGetCurrentContext())
            glfwGetFramebufferSize(window, &backbufferW, &backbufferH);
        glViewport(0, 0, backbufferW, backbufferH);
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader.bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sceneFBO->colorAttachmentID());
        m_shader.setInt("u_Scene", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lightMapID);
        m_shader.setInt("u_Lights", 1);

        m_shader.setFloat("u_Exposure", fx.exposure);
        m_shader.setFloat("u_Vignette", fx.vignette);
        m_shader.setFloat("u_Saturation", fx.saturation);
        m_shader.setInt("u_UseLights", fx.useLights ? 1 : 0);

        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        m_shader.unbind();
    }

    int PostProcessor::width() const { return m_sceneFBO->width(); }
    int PostProcessor::height() const { return m_sceneFBO->height(); }

    void PostProcessor::resize(int w, int h) { m_sceneFBO->resize(w, h); }

} // namespace nebula
