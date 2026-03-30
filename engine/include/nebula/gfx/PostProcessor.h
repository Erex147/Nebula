#pragma once
#include <memory>
#include "nebula/gfx/Framebuffer.h"
#include "nebula/renderer/Shader.h"
#include "nebula/renderer/VertexArray.h"

namespace nebula
{

    struct PostFX
    {
        float exposure = 1.0f;
        float vignette = 0.4f;   // 0 = none, 1 = heavy
        float saturation = 1.0f; // 0 = greyscale, 1 = normal, 2 = vivid
        bool useLights = false;
    };

    class PostProcessor
    {
    public:
        PostProcessor(int w, int h, Shader &screenShader);

        // call before drawing the scene
        void beginScene();
        // call after drawing the scene, before drawing lights
        void endScene();

        // call with the light map texture ID to composite
        void composite(uint32_t lightMapID, const PostFX &fx);

        Framebuffer &sceneFBO() { return *m_sceneFBO; }
        int width() const;
        int height() const;
        void resize(int w, int h);

    private:
        std::unique_ptr<Framebuffer> m_sceneFBO;
        Shader &m_shader;
        uint32_t m_quadVAO = 0;
        uint32_t m_quadVBO = 0;

        void initQuad();
    };

} // namespace nebula