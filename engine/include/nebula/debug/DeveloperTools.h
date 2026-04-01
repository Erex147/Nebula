#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace nebula
{
    class Window;
    class UILayer;
    class DebugOverlay;
    class SceneManager;

    class DeveloperTools
    {
    public:
        void beginFrame(float rawDt, Window &window,
                        DebugOverlay *debug, SceneManager &scenes);
        void draw(UILayer &ui, const Window &window, const SceneManager &scenes);

        bool shouldUpdateSimulation() const;
        float simulationDeltaTime() const { return m_simulationDt; }
        bool overlayVisible() const { return m_overlayVisible; }
        bool paused() const { return m_paused; }
        float timeScale() const { return m_timeScale; }

    private:
        void pushMessage(const std::string &message);

        bool m_overlayVisible = true;
        bool m_paused = false;
        bool m_stepFrame = false;
        float m_timeScale = 1.0f;
        float m_rawDt = 0.0f;
        float m_simulationDt = 0.0f;
        std::vector<std::string> m_messages;
        glm::vec2 m_mouseLogical{0.0f, 0.0f};
    };

} // namespace nebula
