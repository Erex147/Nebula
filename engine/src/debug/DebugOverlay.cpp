#include "nebula/debug/DebugOverlay.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>

namespace nebula
{

    void DebugOverlay::setEnabled(bool enabled)
    {
        m_enabled = enabled;
        visible = enabled;
        if (!m_enabled)
        {
            m_lines.clear();
            m_stats.clear();
        }
    }

    void DebugOverlay::init(const std::string &fontPath, int w, int h)
    {
        init(std::make_shared<FontRenderer>(fontPath, 18), w, h);
    }

    void DebugOverlay::init(std::shared_ptr<FontRenderer> font, int w, int h)
    {
        m_screenW = w;
        m_screenH = h;
        m_font = std::move(font);
        // Y goes from h (top) down to 0 (bottom) — matches FreeType's direction
        m_camera = std::make_unique<Camera2D>(0, (float)w, (float)h, 0);
        m_lastTime = glfwGetTime();
    }

    void DebugOverlay::beginFrame()
    {
        m_lines.clear();

        if (!m_enabled || !m_font)
            return;

        double now = glfwGetTime();
        float dt = (float)(now - m_lastTime);
        m_lastTime = now;

        m_fpsTimer += dt;
        m_frameCount++;
        if (m_fpsTimer >= 0.25f)
        {
            m_fps = (float)m_frameCount / m_fpsTimer;
            m_fpsTimer = 0;
            m_frameCount = 0;
        }
    }

    void DebugOverlay::endFrame(SpriteBatch &batch)
    {
        if (!m_enabled)
        {
            m_stats.clear();
            return;
        }
        if (!visible || !m_font)
        {
            m_stats.clear();
            return;
        }

        batch.begin(m_camera->viewProjection());

        float x = 10.0f;
        float y = (float)m_screenH - 24.0f;
        float lh = m_font->lineHeight();
        auto gray = glm::vec4(0.0f, 0.0f, 0.0f, 0.55f);
        auto white = glm::vec4(1.0f);
        auto green = glm::vec4(0.4f, 1.0f, 0.5f, 1.0f);

        // FPS
        std::ostringstream fps;
        fps << "FPS: " << std::fixed << std::setprecision(1) << m_fps;
        m_font->drawText(batch, fps.str(), x, y, green);
        y -= lh;

        // persistent stats
        for (auto &s : m_stats)
        {
            m_font->drawText(batch, s.key + ": " + s.value, x, y, white);
            y -= lh;
        }

        // one-shot lines
        for (auto &l : m_lines)
        {
            m_font->drawText(batch, l, x, y, white);
            y -= lh;
        }

        batch.flush();
        m_stats.clear();
    }

    void DebugOverlay::print(const std::string &text)
    {
        if (!m_enabled)
            return;
        m_lines.push_back(text);
    }

    void DebugOverlay::setStat(const std::string &k, const std::string &v)
    {
        if (!m_enabled)
            return;
        m_stats.push_back({k, v});
    }
    void DebugOverlay::setStat(const std::string &k, float v)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v;
        setStat(k, ss.str());
    }
    void DebugOverlay::setStat(const std::string &k, int v)
    {
        setStat(k, std::to_string(v));
    }

} // namespace nebula
