#pragma once
#include <string>
#include <glm/glm.hpp>

namespace nebula
{

    // Window
    struct WindowResizedEvent
    {
        int width, height;
    };
    struct WindowClosedEvent
    {
    };

    // Input
    struct KeyPressedEvent
    {
        int keyCode;
    };
    struct KeyReleasedEvent
    {
        int keyCode;
    };
    struct MouseClickEvent
    {
        float x, y;
        int button;
    };
    struct MouseScrollEvent
    {
        float delta;
    };

    // Scene
    struct ScenePushedEvent
    {
        std::string name;
    };
    struct ScenePoppedEvent
    {
        std::string name;
    };

    // Audio
    struct SoundFinishedEvent
    {
        std::string name;
    };

    // Physics
    struct CollisionEvent
    {
        uint32_t entityA;
        uint32_t entityB;
        glm::vec2 normal;
    };

} // namespace nebula