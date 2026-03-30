#include "nebula/renderer/Animator.h"

namespace nebula
{

    Animation Animator::makeAnimation(const std::string &name,
                                      int sw, int sh,
                                      int fw, int fh,
                                      int row, int count,
                                      float dur, bool loop)
    {
        Animation anim;
        anim.name = name;
        anim.loop = loop;

        float invW = 1.0f / (float)sw;
        float invH = 1.0f / (float)sh;

        for (int i = 0; i < count; i++)
        {
            AnimationFrame f;
            f.u0 = (float)(i * fw) * invW;
            f.v0 = (float)(row * fh) * invH;
            f.u1 = (float)((i + 1) * fw) * invW;
            f.v1 = (float)((row + 1) * fh) * invH;
            f.duration = dur;
            anim.frames.push_back(f);
        }
        return anim;
    }

    void Animator::addAnimation(const Animation &anim)
    {
        m_anims[anim.name] = anim;
        if (m_current.empty())
            m_current = anim.name;
    }

    void Animator::play(const std::string &name, bool forceRestart)
    {
        if (m_current == name && !forceRestart)
            return;
        m_current = name;
        m_frameIdx = 0;
        m_timer = 0;
        m_finished = false;
    }

    void Animator::update(float dt)
    {
        if (m_current.empty() || m_finished)
            return;
        auto it = m_anims.find(m_current);
        if (it == m_anims.end())
            return;

        auto &anim = it->second;
        if (anim.frames.empty())
            return;

        m_timer += dt;
        float frameDur = anim.frames[m_frameIdx].duration;

        while (m_timer >= frameDur)
        {
            m_timer -= frameDur;
            m_frameIdx++;
            if (m_frameIdx >= (int)anim.frames.size())
            {
                if (anim.loop)
                {
                    m_frameIdx = 0;
                }
                else
                {
                    m_frameIdx = (int)anim.frames.size() - 1;
                    m_finished = true;
                    return;
                }
            }
            frameDur = anim.frames[m_frameIdx].duration;
        }
    }

    void Animator::draw(SpriteBatch &batch, const Texture &texture,
                        float x, float y, float w, float h,
                        const glm::vec4 &color)
    {
        if (m_current.empty())
            return;
        auto it = m_anims.find(m_current);
        if (it == m_anims.end())
            return;

        auto &frames = it->second.frames;
        if (frames.empty())
            return;

        auto &f = frames[m_frameIdx];
        float u0 = f.u0, u1 = f.u1;
        float v0 = f.v0, v1 = f.v1;

        if (flipX)
            std::swap(u0, u1);
        if (flipY)
            std::swap(v0, v1);

        batch.drawRegion(texture, x, y, w, h, u0, v0, u1, v1, color);
    }

    bool Animator::isPlaying(const std::string &name) const
    {
        return m_current == name && !m_finished;
    }

    bool Animator::isFinished() const { return m_finished; }

} // namespace nebula