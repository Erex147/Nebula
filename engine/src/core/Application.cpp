#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "nebula/core/Application.h"
#include "nebula/core/Input.h"

namespace nebula
{

    Application::Application(const WindowConfig &cfg)
    {
        m_window = std::make_unique<Window>(cfg);
        Input::init(m_window->handle());
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void Application::run()
    {
        onInit();
        double last = glfwGetTime();
        while (m_running && !m_window->shouldClose())
        {
            double now = glfwGetTime();
            float dt = (float)(now - last);
            last = now;

            m_window->pollEvents();
            onUpdate(dt);
            onDraw();
            m_window->swapBuffers();
            Input::endFrame(); // always last
        }
        onShutdown();
        m_assets.clear();
    }

} // namespace nebula