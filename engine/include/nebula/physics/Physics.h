#pragma once
#include "nebula/ecs/World.h"
#include "nebula/ecs/Components.h"
#include <glm/glm.hpp>

namespace nebula
{

    class PhysicsSystem
    {
    public:
        glm::vec2 gravity = {0.0f, -900.0f}; // pixels / s^2

        void update(World &world, float dt);

    private:
        struct AABB
        {
            glm::vec2 min, max;
        };

        AABB getAABB(const Transform &t, const BoxCollider &c) const;
        bool overlaps(const AABB &a, const AABB &b, glm::vec2 &penetration) const;
        void resolve(Transform &ta, RigidBody &ra,
                     Transform &tb, RigidBody &rb,
                     const glm::vec2 &pen);
    };

} // namespace nebula