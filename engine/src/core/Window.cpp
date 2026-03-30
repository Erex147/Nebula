#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "nebula/core/Window.h"

namespace nebula
{

    Window::Window(const WindowConfig &cfg)
        : m_width(cfg.width), m_height(cfg.height)
    {
        if (!glfwInit())
            throw std::runtime_error("Failed to init GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        m_handle = glfwCreateWindow(m_width, m_height,
                                    cfg.title.c_str(), nullptr, nullptr);
        if (!m_handle)
            throw std::runtime_error("Failed to create window");

        glfwMakeContextCurrent(m_handle);
        glfwSwapInterval(1); // vsync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Failed to load OpenGL via GLAD");
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
    }

    bool Window::shouldClose() const
    {
        return glfwWindowShouldClose(m_handle);
    }

    void Window::pollEvents() { glfwPollEvents(); }
    void Window::swapBuffers() { glfwSwapBuffers(m_handle); }

} // namespace nebula