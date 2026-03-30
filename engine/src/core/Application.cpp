#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "nebula/core/Application.h"
#include "nebula/core/Input.h"
#include "nebula/events/EventBus.h"

namespace nebula
{

    static Application *s_instance = nullptr;
    Application &App() { return *s_instance; }

    Application::Application(const WindowConfig &cfg)
    {
        s_instance = this;
        m_window = std::make_unique<Window>(cfg);
        Input::init(m_window->handle());
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // engine/src/core/Application.cpp
    void Application::run()
    {
        onInit(); // game calls debug().init() here if it wants it

        double last = glfwGetTime();
        while (m_running && !m_window->shouldClose())
        {
            double now = glfwGetTime();
            float dt = std::min((float)(now - last), 0.05f);
            last = now;

            m_debug.beginFrame();
            m_window->pollEvents();
            if (Input::keyPressed(Key::F11))
                m_window->toggleFullscreen();
            m_ui.beginFrame(*m_window);
            EventBus::flushDeferred();
            m_scenes.update(dt);
            m_scenes.draw();
            if (m_debugBatch)
                m_ui.render(*m_debugBatch);
            if (m_debugBatch)
                m_debug.endFrame(*m_debugBatch);
            m_window->swapBuffers();
            m_scenes.applyPending();
            m_audio.update();
            Input::endFrame();
        }

        onShutdown();
        m_assets.clear();
    }

} // namespace nebula
