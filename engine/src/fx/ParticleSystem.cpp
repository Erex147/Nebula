#include "nebula/fx/ParticleSystem.h"
#include <cstdlib>
#include <cmath>

namespace nebula
{

    static float randF() { return (float)rand() / (float)RAND_MAX; }
    static float randRange(float lo, float hi) { return lo + randF() * (hi - lo); }

    ParticleSystem::ParticleSystem(size_t max) : m_pool(max) {}

    void ParticleSystem::emit(const ParticleProps &p)
    {
        Particle &particle = m_pool[m_head % m_pool.size()];
        m_head++;

        particle.active = true;
        particle.position = p.position;
        particle.lifeTime = p.lifeTime;
        particle.lifeRemaining = p.lifeTime;
        particle.colorBegin = p.colorBegin;
        particle.colorEnd = p.colorEnd;

        particle.velocity = p.velocity + glm::vec2{
                                             randRange(-p.velocityVariance.x, p.velocityVariance.x),
                                             randRange(-p.velocityVariance.y, p.velocityVariance.y),
                                         };

        float sv = randRange(-p.sizeVariance, p.sizeVariance);
        particle.sizeBegin = p.sizeBegin + sv;
        particle.sizeEnd = p.sizeEnd;
    }

    void ParticleSystem::update(float dt)
    {
        for (auto &p : m_pool)
        {
            if (!p.active)
                continue;
            p.lifeRemaining -= dt;
            if (p.lifeRemaining <= 0.0f)
            {
                p.active = false;
                continue;
            }
            p.position += p.velocity * dt;
            // slight drag
            p.velocity *= (1.0f - 2.0f * dt);
        }
    }

    void ParticleSystem::draw(SpriteBatch &batch, const Texture &tex)
    {
        for (auto &p : m_pool)
        {
            if (!p.active)
                continue;
            float t = 1.0f - (p.lifeRemaining / p.lifeTime);
            glm::vec4 c = glm::mix(p.colorBegin, p.colorEnd, t);
            float size = glm::mix(p.sizeBegin, p.sizeEnd, t);
            batch.draw(tex,
                       p.position.x - size * 0.5f,
                       p.position.y - size * 0.5f,
                       size, size, c);
        }
    }

    size_t ParticleSystem::activeCount() const
    {
        size_t n = 0;
        for (auto &p : m_pool)
            if (p.active)
                n++;
        return n;
    }

} // namespace nebula