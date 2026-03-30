#include "nebula/core/Input.h"
#include "nebula/events/EventBus.h"
#include "nebula/events/Events.h"
#include <GLFW/glfw3.h>

namespace nebula {

std::array<bool, Input::KEY_COUNT> Input::s_keysCur    = {};
std::array<bool, Input::KEY_COUNT> Input::s_keysPrev   = {};
std::array<bool, Input::BTN_COUNT> Input::s_btnCur     = {};
std::array<bool, Input::BTN_COUNT> Input::s_btnPrev    = {};
glm::vec2 Input::s_mousePos     = {};
glm::vec2 Input::s_mousePosPrev = {};
float     Input::s_scrollY      = 0.0f;

void Input::init(GLFWwindow* w) {
    glfwSetKeyCallback         (w, cbKey);
    glfwSetMouseButtonCallback (w, cbBtn);
    glfwSetCursorPosCallback   (w, cbCursor);
    glfwSetScrollCallback      (w, cbScroll);
}

bool Input::keyDown    (Key k) { return s_keysCur[(int)k]; }
bool Input::keyPressed (Key k) { return  s_keysCur[(int)k] && !s_keysPrev[(int)k]; }
bool Input::keyReleased(Key k) { return !s_keysCur[(int)k] &&  s_keysPrev[(int)k]; }

bool Input::mouseDown   (MouseButton b) { return s_btnCur[(int)b]; }
bool Input::mousePressed(MouseButton b) { return s_btnCur[(int)b] && !s_btnPrev[(int)b]; }

glm::vec2 Input::mousePos()    { return s_mousePos; }
glm::vec2 Input::mouseDelta()  { return s_mousePos - s_mousePosPrev; }
float     Input::scrollDelta() { return s_scrollY; }

void Input::endFrame() {
    s_keysPrev    = s_keysCur;
    s_btnPrev     = s_btnCur;
    s_mousePosPrev = s_mousePos;
    s_scrollY     = 0.0f;
}

void Input::cbKey(GLFWwindow*, int k, int, int action, int) {
    if (k >= 0 && k < KEY_COUNT)
    {
        s_keysCur[k] = (action != GLFW_RELEASE);
        if (action == GLFW_PRESS)
            EventBus::defer(KeyPressedEvent{k});
        else if (action == GLFW_RELEASE)
            EventBus::defer(KeyReleasedEvent{k});
    }
}
void Input::cbBtn(GLFWwindow*, int b, int action, int) {
    if (b >= 0 && b < BTN_COUNT)
    {
        s_btnCur[b] = (action != GLFW_RELEASE);
        if (action == GLFW_PRESS)
            EventBus::defer(MouseClickEvent{s_mousePos.x, s_mousePos.y, b});
    }
}
void Input::cbCursor(GLFWwindow*, double x, double y) {
    s_mousePos = {(float)x, (float)y};
}
void Input::cbScroll(GLFWwindow*, double, double y) {
    s_scrollY = (float)y;
    EventBus::defer(MouseScrollEvent{(float)y});
}

} // namespace nebula
