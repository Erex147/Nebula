#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "nebula/gfx/Framebuffer.h"
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/renderer/Shader.h"
#include "nebula/math/Camera2D.h"

namespace nebula
{

    struct PointLight
    {
        glm::vec2 position;
        glm::vec3 color = {1.0f, 0.9f, 0.7f};
        float radius = 200.0f;
        float intensity = 1.0f;
    };

    class LightRenderer
    {
    public:
        glm::vec3 ambientColor = {0.05f, 0.05f, 0.08f}; // very dark blue
        float ambientStrength = 0.08f;

        LightRenderer(int w, int h, Shader &lightShader);

        void begin(const glm::mat4 &viewProjection);
        void submitLight(const PointLight &light);
        void end();

        uint32_t lightMapID() const;
        void resize(int w, int h);

    private:
        std::unique_ptr<Framebuffer> m_lightFBO;
        std::unique_ptr<SpriteBatch> m_batch;
    };

} // namespace nebula