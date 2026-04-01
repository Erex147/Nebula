#include "nebula/debug/DeveloperTools.h"
#include "nebula/core/Input.h"
#include "nebula/core/KeyCodes.h"
#include "nebula/core/Window.h"
#include "nebula/debug/DebugOverlay.h"
#include "nebula/scene/SceneManager.h"
#include "nebula/ui/UILayer.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace nebula
{

    void DeveloperTools::beginFrame(float rawDt, Window &window,
                                    DebugOverlay *debug, SceneManager &scenes)
    {
        m_rawDt = rawDt;
        m_mouseLogical = window.windowToLogical(Input::mousePos());

        if (Input::keyPressed(Key::F7))
        {
            if (debug)
            {
                debug->visible = !debug->visible;
                pushMessage(debug->visible ? "Debug overlay enabled" : "Debug overlay hidden");
            }
            else
            {
                pushMessage("Debug overlay not installed");
            }
        }
        if (Input::keyPressed(Key::F8))
        {
            m_paused = !m_paused;
            pushMessage(m_paused ? "Simulation paused" : "Simulation resumed");
        }
        if (Input::keyPressed(Key::F9))
        {
            m_stepFrame = true;
            pushMessage("Step frame");
        }
        if (Input::keyPressed(Key::F10))
        {
            m_overlayVisible = !m_overlayVisible;
        }
        if (Input::keyPressed(Key::Num1))
        {
            m_timeScale = 0.25f;
            pushMessage("Time scale 0.25x");
        }
        if (Input::keyPressed(Key::Num2))
        {
            m_timeScale = 0.5f;
            pushMessage("Time scale 0.5x");
        }
        if (Input::keyPressed(Key::Num3))
        {
            m_timeScale = 1.0f;
            pushMessage("Time scale 1.0x");
        }
        if (Input::keyPressed(Key::Num4))
        {
            m_timeScale = 2.0f;
            pushMessage("Time scale 2.0x");
        }

        if (m_paused && !m_stepFrame)
            m_simulationDt = 0.0f;
        else
            m_simulationDt = rawDt * m_timeScale;

        if (scenes.empty())
            pushMessage("Scene stack empty");
    }

    void DeveloperTools::draw(UILayer &ui, const Window &window, const SceneManager &scenes)
    {
        if (!m_overlayVisible)
            return;

        const UILabelStyle titleStyle{glm::vec4{0.95f, 0.98f, 1.0f, 1.0f}, 0.95f};
        const UILabelStyle bodyStyle{glm::vec4{0.88f, 0.92f, 0.98f, 1.0f}, 0.74f};
        const UILabelStyle hintStyle{glm::vec4{0.74f, 0.82f, 0.90f, 0.94f}, 0.68f};
        const UILabelStyle eventStyle{glm::vec4{0.88f, 0.92f, 0.98f, 1.0f}, 0.72f};
        const glm::vec4 stateColor = m_paused
                                         ? glm::vec4{1.0f, 0.86f, 0.52f, 1.0f}
                                         : glm::vec4{0.72f, 1.0f, 0.82f, 1.0f};
        const UILabelStyle stateStyle{stateColor, 0.78f};

        UIRect card = ui.anchoredRect({300.0f, 186.0f}, UIAnchor::TopRight, {-20.0f, 20.0f});
        ui.panel(card, {{0.03f, 0.05f, 0.08f, 0.78f}, {0.9f, 0.95f, 1.0f, 0.16f}, 2.0f});
        auto layout = ui.stack(ui.insetRect(card, {16.0f, 16.0f, 16.0f, 14.0f}), UIAxis::Vertical, 8.0f);

        ui.label("Dev Tools", layout.next({250.0f, 18.0f}).position, titleStyle);

        std::ostringstream rawDtText;
        rawDtText << std::fixed << std::setprecision(3) << (m_rawDt * 1000.0f) << " ms";
        std::ostringstream simDtText;
        simDtText << std::fixed << std::setprecision(3) << (m_simulationDt * 1000.0f) << " ms";
        std::ostringstream timeScaleText;
        timeScaleText << std::fixed << std::setprecision(2) << m_timeScale << "x";

        ui.label("State " + std::string(m_paused ? "Paused" : "Running"), layout.next({250.0f, 16.0f}).position, stateStyle);
        ui.label("Raw dt " + rawDtText.str(), layout.next({250.0f, 16.0f}).position, bodyStyle);
        ui.label("Sim dt " + simDtText.str(), layout.next({250.0f, 16.0f}).position, bodyStyle);
        ui.label("Time scale " + timeScaleText.str(), layout.next({250.0f, 16.0f}).position, bodyStyle);
        ui.label("Scenes " + std::to_string((int)scenes.stackSize()) + " / pending " + std::to_string((int)scenes.pendingCount()),
                 layout.next({250.0f, 16.0f}).position, bodyStyle);
        ui.label("Mouse " + std::to_string((int)m_mouseLogical.x) + ", " + std::to_string((int)m_mouseLogical.y),
                 layout.next({250.0f, 16.0f}).position, bodyStyle);
        ui.label("Window " + std::to_string(window.width()) + "x" + std::to_string(window.height()),
                 layout.next({250.0f, 16.0f}).position, bodyStyle);
        ui.label("F7 debug  F8 pause  F9 step  F10 tools", layout.next({260.0f, 16.0f}).position,
                 hintStyle);
        ui.label("1/2/3/4 = 0.25x / 0.5x / 1x / 2x", layout.next({260.0f, 16.0f}).position,
                 hintStyle);

        auto messagesRect = ui.anchoredRect({320.0f, 92.0f}, UIAnchor::BottomRight, {-20.0f, -20.0f});
        ui.panel(messagesRect, {{0.03f, 0.05f, 0.08f, 0.66f}, {0.9f, 0.95f, 1.0f, 0.12f}, 2.0f});
        auto messages = ui.stack(ui.insetRect(messagesRect, {14.0f, 12.0f, 14.0f, 10.0f}), UIAxis::Vertical, 6.0f);
        ui.label("Events", messages.next({220.0f, 16.0f}).position, UILabelStyle{titleStyle.color, 0.84f});
        for (const auto &message : m_messages)
        {
            ui.label(message, messages.next({280.0f, 14.0f}).position, eventStyle);
        }

        m_stepFrame = false;
    }

    bool DeveloperTools::shouldUpdateSimulation() const
    {
        return !m_paused || m_stepFrame;
    }

    void DeveloperTools::pushMessage(const std::string &message)
    {
        m_messages.push_back(message);
        if (m_messages.size() > 4)
            m_messages.erase(m_messages.begin());
    }

} // namespace nebula
