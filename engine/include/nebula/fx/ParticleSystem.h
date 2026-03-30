#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/renderer/Texture.h"

namespace nebula
{

    struct ParticleProps
    {
        glm::vec2 position = {0, 0};
        glm::vec2 velocity = {0, 0};
        glm::vec2 velocityVariance = {50, 50}; // ± random spread
        glm::vec4 colorBegin = {1, 1, 0, 1};
        glm::vec4 colorEnd = {1, 0, 0, 0};
        float sizeBegin = 16.0f;
        float sizeEnd = 2.0f;
        float sizeVariance = 4.0f;
        float lifeTime = 1.0f; // seconds
    };

    class ParticleSystem
    {
    public:
        explicit ParticleSystem(size_t maxParticles = 2000);

        void emit(const ParticleProps &props);
        void update(float dt);
        void draw(SpriteBatch &batch, const Texture &texture);

        size_t activeCount() const;

    private:
        struct Particle
        {
            glm::vec2 position;
            glm::vec2 velocity;
            glm::vec4 colorBegin, colorEnd;
            float sizeBegin, sizeEnd;
            float lifeTime, lifeRemaining;
            bool active = false;
        };

        std::vector<Particle> m_pool;
        size_t m_head = 0; // cycles through pool (ring buffer)
    };

} // namespace nebula