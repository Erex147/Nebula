#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "nebula/core/Window.h"
#include "nebula/events/EventBus.h"
#include "nebula/events/Events.h"

namespace nebula
{

    Window::Window(const WindowConfig &cfg)
        : m_width(cfg.width), m_height(cfg.height),
          m_logicalWidth(cfg.logicalWidth > 0 ? cfg.logicalWidth : cfg.width),
          m_logicalHeight(cfg.logicalHeight > 0 ? cfg.logicalHeight : cfg.height),
          m_windowedWidth(cfg.width), m_windowedHeight(cfg.height),
          m_fullscreen(cfg.startFullscreen)
    {
        if (!glfwInit())
            throw std::runtime_error("Failed to init GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        GLFWmonitor *monitor = nullptr;
        if (cfg.startFullscreen)
        {
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            m_width = mode->width;
            m_height = mode->height;
        }

        m_handle = glfwCreateWindow(m_width, m_height,
                                    cfg.title.c_str(), monitor, nullptr);
        if (!m_handle)
            throw std::runtime_error("Failed to create window");

        glfwMakeContextCurrent(m_handle);
        glfwSwapInterval(1); // vsync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Failed to load OpenGL via GLAD");

        glfwSetWindowUserPointer(m_handle, this);
        glfwSetWindowSizeCallback(
            m_handle,
            [](GLFWwindow *window, int width, int height)
            {
                auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
                if (!self)
                    return;
                self->m_width = width;
                self->m_height = height;
                EventBus::defer(WindowResizedEvent{width, height});
            });
        glfwSetFramebufferSizeCallback(
            m_handle,
            [](GLFWwindow *window, int width, int height)
            {
                auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
                if (!self)
                    return;
                self->m_fbWidth = width;
                self->m_fbHeight = height;
                glViewport(0, 0, width, height);
            });
        glfwSetWindowCloseCallback(
            m_handle,
            [](GLFWwindow *)
            {
                EventBus::defer(WindowClosedEvent{});
            });

        glfwGetWindowPos(m_handle, &m_windowedX, &m_windowedY);
        glfwGetFramebufferSize(m_handle, &m_fbWidth, &m_fbHeight);
        glViewport(0, 0, m_fbWidth, m_fbHeight);
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

    void Window::applyWindowedSize(int width, int height)
    {
        m_width = width;
        m_height = height;
        glfwSetWindowMonitor(m_handle, nullptr, m_windowedX, m_windowedY, width, height, 0);
    }

    void Window::toggleFullscreen()
    {
        if (!m_handle)
            return;

        if (m_fullscreen)
        {
            m_fullscreen = false;
            applyWindowedSize(m_windowedWidth, m_windowedHeight);
            return;
        }

        glfwGetWindowPos(m_handle, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_handle, &m_windowedWidth, &m_windowedHeight);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        m_fullscreen = true;
        m_width = mode->width;
        m_height = mode->height;
        glfwSetWindowMonitor(m_handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    glm::vec2 Window::windowToLogical(const glm::vec2 &windowPos) const
    {
        float sx = m_width > 0 ? (float)m_logicalWidth / (float)m_width : 1.0f;
        float sy = m_height > 0 ? (float)m_logicalHeight / (float)m_height : 1.0f;
        return {windowPos.x * sx, windowPos.y * sy};
    }

    glm::vec2 Window::windowToLogicalBottomLeft(const glm::vec2 &windowPos) const
    {
        glm::vec2 logical = windowToLogical(windowPos);
        logical.y = (float)m_logicalHeight - logical.y;
        return logical;
    }

    glm::vec2 Window::logicalToWindow(const glm::vec2 &logicalPos) const
    {
        float sx = m_logicalWidth > 0 ? (float)m_width / (float)m_logicalWidth : 1.0f;
        float sy = m_logicalHeight > 0 ? (float)m_height / (float)m_logicalHeight : 1.0f;
        return {logicalPos.x * sx, logicalPos.y * sy};
    }

    glm::vec2 Window::logicalToWindowBottomLeft(const glm::vec2 &logicalPos) const
    {
        glm::vec2 topLeft{logicalPos.x, (float)m_logicalHeight - logicalPos.y};
        return logicalToWindow(topLeft);
    }

} // namespace nebula
