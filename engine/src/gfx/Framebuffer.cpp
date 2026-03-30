#include "nebula/gfx/Framebuffer.h"
#include <glad/glad.h>
#include <stdexcept>

namespace nebula
{

    Framebuffer::Framebuffer(const FramebufferSpec &spec) : m_spec(spec)
    {
        create();
    }

    Framebuffer::~Framebuffer() { destroy(); }

    void Framebuffer::create()
    {
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // color attachment
        glGenTextures(1, &m_colorID);
        glBindTexture(GL_TEXTURE_2D, m_colorID);
        GLenum internalFmt = m_spec.hdr ? GL_RGBA16F : GL_RGBA8;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFmt,
                     m_spec.width, m_spec.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_colorID, 0);

        // optional depth attachment
        if (m_spec.hasDepth)
        {
            glGenRenderbuffers(1, &m_depthID);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depthID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                                  m_spec.width, m_spec.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_STENCIL_ATTACHMENT,
                                      GL_RENDERBUFFER, m_depthID);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Framebuffer is not complete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_colorTexture = Texture::fromID(m_colorID, m_spec.width, m_spec.height,
                                         TextureLoadOptions::renderTarget());
    }

    void Framebuffer::destroy()
    {
        m_colorTexture.reset();
        if (m_colorID)
            glDeleteTextures(1, &m_colorID);
        if (m_depthID)
            glDeleteRenderbuffers(1, &m_depthID);
        if (m_fbo)
            glDeleteFramebuffers(1, &m_fbo);
        m_colorID = m_depthID = m_fbo = 0;
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_spec.width, m_spec.height);
    }

    void Framebuffer::unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::clear(float r, float g, float b, float a) const
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT |
                (m_spec.hasDepth ? GL_DEPTH_BUFFER_BIT : 0));
    }

    void Framebuffer::resize(int w, int h)
    {
        m_spec.width = w;
        m_spec.height = h;
        destroy();
        create();
    }

} // namespace nebula
