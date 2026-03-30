#pragma once
#include <cstdint>
#include <memory>
#include "nebula/renderer/Texture.h"

namespace nebula
{

    struct FramebufferSpec
    {
        int width = 1280;
        int height = 720;
        bool hasDepth = false;
        bool hdr = false; // use GL_RGBA16F instead of GL_RGBA8
    };

    class Framebuffer
    {
    public:
        explicit Framebuffer(const FramebufferSpec &spec);
        ~Framebuffer();

        // bind — subsequent draw calls render into this framebuffer
        void bind() const;
        // unbind — draw calls go back to the screen
        void unbind() const;

        void clear(float r = 0, float g = 0, float b = 0, float a = 1) const;

        // resize (e.g. on window resize)
        void resize(int w, int h);

        uint32_t colorAttachmentID() const { return m_colorID; }
        uint32_t depthAttachmentID() const { return m_depthID; }
        int width() const { return m_spec.width; }
        int height() const { return m_spec.height; }

        // convenience: get color attachment as a Texture for sampling
        // the returned texture shares the GL id — do not delete it
        const Texture &colorTexture() const { return *m_colorTexture; }

    private:
        FramebufferSpec m_spec;
        uint32_t m_fbo = 0;
        uint32_t m_colorID = 0;
        uint32_t m_depthID = 0;
        std::unique_ptr<Texture> m_colorTexture;

        void create();
        void destroy();
    };

} // namespace nebula