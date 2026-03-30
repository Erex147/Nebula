#include "nebula/core/Application.h"
#include "nebula/core/Input.h"
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/renderer/RenderCommand.h"
#include "nebula/renderer/Tilemap.h"
#include "nebula/math/Camera2D.h"
#include "nebula/ecs/World.h"
#include "nebula/ecs/Components.h"
#include "nebula/fx/ParticleSystem.h"
#include "nebula/physics/Physics.h"
#include <memory>

using namespace nebula;

class Game : public Application
{
public:
    Game() : Application({"Game", 1280, 720}) {}

    void onInit() override
    {
        auto shader = assets().loadShader("sprite",
                                          ASSETS_PATH "shaders/sprite.vert",
                                          ASSETS_PATH "shaders/sprite.frag");

        m_tex = assets().loadTexture(ASSETS_PATH "textures/test.png");
        m_batch = std::make_unique<SpriteBatch>(*shader);
        m_camera = std::make_unique<Camera2D>(0, 1280, 0, 720);
        m_tilemap = std::make_unique<Tilemap>(20, 12, 64, 64, 1);
        m_particles = std::make_unique<ParticleSystem>(2000);

        // build a simple ground and some platforms
        m_tilemap->setAtlas(m_tex);
        for (int x = 0; x < 20; x++)
        {
            m_tilemap->setTile(x, 0, 0); // floor
            m_tilemap->setTile(x, 1, 0);
        }
        for (int x = 4; x < 8; x++)
            m_tilemap->setTile(x, 4, 0); // platform
        for (int x = 12; x < 16; x++)
            m_tilemap->setTile(x, 6, 0); // platform

        // player entity
        m_player = m_world.create();
        m_world.add<Transform>(m_player, Transform{{200, 300}});
        m_world.add<Sprite>(m_player, Sprite{m_tex, {48, 48}, {0.4f, 0.8f, 1.0f, 1.0f}});
        m_world.add<RigidBody>(m_player);
        m_world.add<BoxCollider>(m_player, BoxCollider{{48, 48}});
        m_world.add<Tag>(m_player, Tag{"player"});

        // static floor colliders matching the tilemap
        for (int x = 0; x < 20; x++)
        {
            EntityID tile = m_world.create();
            m_world.add<Transform>(tile, Transform{{(float)(x * 64), 0}});
            RigidBody rb;
            rb.isStatic = true;
            m_world.add<RigidBody>(tile, rb);
            m_world.add<BoxCollider>(tile, BoxCollider{{64, 128}});
        }
        for (int x = 4; x < 8; x++)
        {
            EntityID tile = m_world.create();
            m_world.add<Transform>(tile, Transform{{(float)(x * 64), 256}});
            RigidBody rb;
            rb.isStatic = true;
            m_world.add<RigidBody>(tile, rb);
            m_world.add<BoxCollider>(tile, BoxCollider{{64, 64}});
        }
        for (int x = 12; x < 16; x++)
        {
            EntityID tile = m_world.create();
            m_world.add<Transform>(tile, Transform{{(float)(x * 64), 384}});
            RigidBody rb;
            rb.isStatic = true;
            m_world.add<RigidBody>(tile, rb);
            m_world.add<BoxCollider>(tile, BoxCollider{{64, 64}});
        }
    }

    void onUpdate(float dt) override
    {
        auto &t = m_world.get<Transform>(m_player);
        auto &rb = m_world.get<RigidBody>(m_player);

        // horizontal movement
        if (Input::keyDown(Key::A) || Input::keyDown(Key::Left))
            rb.velocity.x = -300.0f;
        else if (Input::keyDown(Key::D) || Input::keyDown(Key::Right))
            rb.velocity.x = 300.0f;
        else
            rb.velocity.x *= 0.8f; // friction

        // jump
        if ((Input::keyPressed(Key::Space) || Input::keyPressed(Key::Up)) && rb.onGround)
            rb.velocity.y = 550.0f;

        // escape to quit
        if (Input::keyPressed(Key::Escape))
            quit();

        // particles on spacebar hold
        if (Input::keyDown(Key::Space))
        {
            ParticleProps pp;
            pp.position = t.position + glm::vec2{24, 24};
            pp.velocity = {0, -80};
            pp.velocityVariance = {120, 60};
            pp.colorBegin = {1.0f, 0.8f, 0.2f, 1.0f};
            pp.colorEnd = {1.0f, 0.2f, 0.0f, 0.0f};
            pp.sizeBegin = 12;
            pp.sizeEnd = 2;
            pp.lifeTime = 0.5f;
            for (int i = 0; i < 3; i++)
                m_particles->emit(pp);
        }

        m_physics.update(m_world, dt);
        m_particles->update(dt);

        // camera follows player
        glm::vec2 target = t.position - glm::vec2{640, 360};
        m_camera->setPosition(glm::mix(m_camera->position(), target, 8.0f * dt));
    }

    void onDraw() override
    {
        RenderCommand::setClearColor({0.08f, 0.08f, 0.12f, 1.0f});
        RenderCommand::clear();

        m_batch->begin(m_camera->viewProjection());

        // tilemap
        m_tilemap->draw(*m_batch);

        // all entities with Transform + Sprite
        m_world.view<Transform, Sprite>().each(
            [&](EntityID, Transform &t, Sprite &s)
            {
                m_batch->draw(*s.texture,
                              t.position.x, t.position.y,
                              s.size.x, s.size.y, s.color);
            });

        // particles
        m_particles->draw(*m_batch, *m_tex);

        m_batch->flush();
    }

private:
    World m_world;
    PhysicsSystem m_physics;
    EntityID m_player = NULL_ENTITY;

    std::unique_ptr<SpriteBatch> m_batch;
    std::unique_ptr<Camera2D> m_camera;
    std::unique_ptr<Tilemap> m_tilemap;
    std::unique_ptr<ParticleSystem> m_particles;
    std::shared_ptr<Texture> m_tex;
};

int main()
{
    Game game;
    game.run();
}