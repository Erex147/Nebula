#include "nebula/physics/Physics.h"
#include <algorithm>
#include <cmath>

namespace nebula
{

    void PhysicsSystem::update(World &world, float dt)
    {
        // 1. apply gravity and integrate
        world.view<Transform, RigidBody>().each(
            [&](EntityID, Transform &t, RigidBody &rb)
            {
                if (rb.isStatic)
                    return;
                rb.velocity += (gravity + rb.acceleration) * dt;
                t.position += rb.velocity * dt;
                rb.onGround = false;
            });

        // 2. resolve collisions between all pairs with BoxCollider
        // collect entities first to avoid iterator invalidation
        std::vector<EntityID> collidables;
        world.view<Transform, RigidBody, BoxCollider>().each(
            [&](EntityID e, Transform &, RigidBody &, BoxCollider &)
            {
                collidables.push_back(e);
            });

        for (size_t i = 0; i < collidables.size(); i++)
        {
            for (size_t j = i + 1; j < collidables.size(); j++)
            {
                EntityID ea = collidables[i];
                EntityID eb = collidables[j];

                auto &ta = world.get<Transform>(ea);
                auto &ra = world.get<RigidBody>(ea);
                auto &ca = world.get<BoxCollider>(ea);
                auto &tb = world.get<Transform>(eb);
                auto &rb = world.get<RigidBody>(eb);
                auto &cb = world.get<BoxCollider>(eb);

                if (ra.isStatic && rb.isStatic)
                    continue;

                AABB aabb_a = getAABB(ta, ca);
                AABB aabb_b = getAABB(tb, cb);
                glm::vec2 pen;
                if (overlaps(aabb_a, aabb_b, pen))
                    resolve(ta, ra, tb, rb, pen);
            }
        }
    }

    PhysicsSystem::AABB PhysicsSystem::getAABB(const Transform &t,
                                               const BoxCollider &c) const
    {
        glm::vec2 origin = t.position + c.offset;
        return {origin, origin + c.size};
    }

    bool PhysicsSystem::overlaps(const AABB &a, const AABB &b,
                                 glm::vec2 &pen) const
    {
        glm::vec2 aHalf = (a.max - a.min) * 0.5f;
        glm::vec2 bHalf = (b.max - b.min) * 0.5f;
        glm::vec2 delta = (a.min + aHalf) - (b.min + bHalf);

        float ox = aHalf.x + bHalf.x - fabsf(delta.x);
        float oy = aHalf.y + bHalf.y - fabsf(delta.y);

        if (ox <= 0 || oy <= 0)
            return false;

        // push out along the axis of least penetration
        if (ox < oy)
            pen = {(delta.x < 0 ? -ox : ox), 0};
        else
            pen = {0, (delta.y < 0 ? -oy : oy)};

        return true;
    }

    void PhysicsSystem::resolve(Transform &ta, RigidBody &ra,
                                Transform &tb, RigidBody &rb,
                                const glm::vec2 &pen)
    {
        if (rb.isStatic)
        {
            ta.position += pen;
            if (pen.y > 0)
            {
                ra.velocity.y = 0;
                ra.onGround = true;
            }
            if (pen.y < 0)
                ra.velocity.y = 0;
            if (pen.x != 0)
                ra.velocity.x = 0;
        }
        else if (ra.isStatic)
        {
            tb.position -= pen;
            if (pen.y < 0)
            {
                rb.velocity.y = 0;
                rb.onGround = true;
            }
            if (pen.y > 0)
                rb.velocity.y = 0;
            if (pen.x != 0)
                rb.velocity.x = 0;
        }
        else
        {
            // split correction by mass
            float total = ra.mass + rb.mass;
            ta.position += pen * (rb.mass / total);
            tb.position -= pen * (ra.mass / total);
            // simple inelastic response
            glm::vec2 relVel = ra.velocity - rb.velocity;
            glm::vec2 n = glm::normalize(pen);
            float vn = glm::dot(relVel, n);
            if (vn > 0)
                return;
            float impulse = -vn / total;
            ra.velocity += impulse * rb.mass * n;
            rb.velocity -= impulse * ra.mass * n;
        }
    }

} // namespace nebula