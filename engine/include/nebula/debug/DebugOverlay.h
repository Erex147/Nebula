#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "nebula/renderer/FontRenderer.h"
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/math/Camera2D.h"

namespace nebula
{

    class DebugOverlay
    {
    public:
        bool visible = true;

        void init(const std::string &fontPath,
                  int screenW, int screenH);
        void init(std::shared_ptr<FontRenderer> font,
                  int screenW, int screenH);
        void setEnabled(bool enabled);
        bool enabled() const { return m_enabled; }

        // call at start of frame
        void beginFrame();
        // call at end of frame — draws everything
        void endFrame(SpriteBatch &batch);

        // add a line of text visible for one frame
        void print(const std::string &text);

        // persistent named stats updated every frame
        void setStat(const std::string &key, const std::string &value);
        void setStat(const std::string &key, float value);
        void setStat(const std::string &key, int value);

    private:
        std::shared_ptr<FontRenderer> m_font;
        std::unique_ptr<Camera2D> m_camera;

        struct Stat
        {
            std::string key, value;
        };
        std::vector<std::string> m_lines;
        std::vector<Stat> m_stats;

        // fps tracking
        float m_fpsTimer = 0;
        int m_frameCount = 0;
        float m_fps = 0;
        double m_lastTime = 0;
        int m_screenW = 0;
        int m_screenH = 0;
        bool m_enabled = true;
    };

} // namespace nebula
