#pragma once
#include <string>

struct GLFWwindow;

namespace nebula
{

    struct WindowConfig
    {
        std::string title = "Nebula";
        int width = 1280;
        int height = 720;
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
        GLFWwindow* handle() const { return m_handle; }

    private:
        GLFWwindow *m_handle = nullptr;
        int m_width, m_height;
    };

} // namespace nebula