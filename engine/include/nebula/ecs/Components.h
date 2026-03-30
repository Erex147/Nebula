#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "nebula/renderer/Texture.h"

namespace nebula
{

    struct Transform
    {
        glm::vec2 position = {0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
        float rotation = 0.0f; // radians
    };

    struct Sprite
    {
        std::shared_ptr<Texture> texture;
        glm::vec2 size = {64.0f, 64.0f};
        glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    };

    struct Velocity
    {
        glm::vec2 value = {0.0f, 0.0f};
    };

    struct Tag
    {
        std::string name;
    };

    struct RigidBody
    {
        glm::vec2 velocity = {0.0f, 0.0f};
        glm::vec2 acceleration = {0.0f, 0.0f};
        float mass = 1.0f;
        bool isStatic = false;
        bool onGround = false;
    };

    struct BoxCollider
    {
        glm::vec2 size = {64.0f, 64.0f}; // full width/height
        glm::vec2 offset = {0.0f, 0.0f};
    };

} // namespace nebula