#pragma once
#include <string>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace nebula
{

    struct WindowConfig
    {
        std::string title = "Nebula";
        int width = 1280;
        int height = 720;
        int logicalWidth = 0;
        int logicalHeight = 0;
        bool startFullscreen = false;
    };

    class Window
    {
    public:
        explicit Window(const WindowConfig &cfg);
        ~Window();

        bool shouldClose() const;
        void pollEvents();
        void swapBuffers();
        int width() const { return m_width; }
        int height() const { return m_height; }
        int logicalWidth() const { return m_logicalWidth; }
        int logicalHeight() const { return m_logicalHeight; }
        int framebufferWidth() const { return m_fbWidth; }
        int framebufferHeight() const { return m_fbHeight; }
        glm::vec2 logicalSize() const { return {(float)m_logicalWidth, (float)m_logicalHeight}; }
        glm::vec2 windowToLogical(const glm::vec2 &windowPos) const;
        glm::vec2 windowToLogicalBottomLeft(const glm::vec2 &windowPos) const;
        glm::vec2 logicalToWindow(const glm::vec2 &logicalPos) const;
        glm::vec2 logicalToWindowBottomLeft(const glm::vec2 &logicalPos) const;
        GLFWwindow* handle() const { return m_handle; }
        bool isFullscreen() const { return m_fullscreen; }
        void toggleFullscreen();

    private:
        void applyWindowedSize(int width, int height);

        GLFWwindow *m_handle = nullptr;
        int m_width, m_height;
        int m_logicalWidth = 0;
        int m_logicalHeight = 0;
        int m_fbWidth = 0;
        int m_fbHeight = 0;
        bool m_fullscreen = false;
        int m_windowedX = 100;
        int m_windowedY = 100;
        int m_windowedWidth = 1280;
        int m_windowedHeight = 720;
    };

} // namespace nebula
