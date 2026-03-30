#pragma once
#include "nebula/core/KeyCodes.h"
#include <glm/glm.hpp>
#include <array>

struct GLFWwindow;

namespace nebula {

class Input {
public:
    static void init(GLFWwindow* window);

    // held this frame
    static bool keyDown    (Key key);
    // pressed exactly this frame
    static bool keyPressed (Key key);
    // released exactly this frame
    static bool keyReleased(Key key);

    static bool mouseDown   (MouseButton btn);
    static bool mousePressed(MouseButton btn);

    static glm::vec2 mousePos();
    static glm::vec2 mouseDelta();
    static float     scrollDelta();

    // call at the end of every frame
    static void endFrame();

private:
    static constexpr int KEY_COUNT = GLFW_KEY_LAST + 1;
    static constexpr int BTN_COUNT = GLFW_MOUSE_BUTTON_LAST + 1;

    static std::array<bool, KEY_COUNT> s_keysCur;
    static std::array<bool, KEY_COUNT> s_keysPrev;
    static std::array<bool, BTN_COUNT> s_btnCur;
    static std::array<bool, BTN_COUNT> s_btnPrev;
    static glm::vec2 s_mousePos;
    static glm::vec2 s_mousePosPrev;
    static float     s_scrollY;

    static void cbKey   (GLFWwindow*, int k, int, int action, int);
    static void cbBtn   (GLFWwindow*, int b, int action, int);
    static void cbCursor(GLFWwindow*, double x, double y);
    static void cbScroll(GLFWwindow*, double, double y);
};

} // namespace nebula