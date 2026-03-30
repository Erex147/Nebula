#include "nebula/assets/AssetManager.h"
#include "nebula/audio/AudioManager.h"
#include "nebula/core/Application.h"
#include "nebula/core/Input.h"
#include "nebula/core/KeyCodes.h"
#include "nebula/ecs/Components.h"
#include "nebula/ecs/World.h"
#include "nebula/events/EventBus.h"
#include "nebula/events/Events.h"
#include "nebula/events/Signal.h"
#include "nebula/fx/ParticleSystem.h"
#include "nebula/gfx/LightRenderer.h"
#include "nebula/gfx/PostProcessor.h"
#include "nebula/math/Camera2D.h"
#include "nebula/physics/Physics.h"
#include "nebula/renderer/Animator.h"
#include "nebula/renderer/RenderCommand.h"
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/renderer/TextureAtlas.h"
#include "nebula/renderer/Tilemap.h"
#include "nebula/scene/Scene.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <memory>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace nebula;

namespace
{
    constexpr int SCREEN_W = 1280;
    constexpr int SCREEN_H = 720;
    constexpr int TILE_SIZE = 64;
    constexpr int MAP_W = 28;
    constexpr int MAP_H = 12;
    constexpr float WORLD_W = (float)(MAP_W * TILE_SIZE);
    constexpr ListenerID INVALID_LISTENER = std::numeric_limits<ListenerID>::max();

    struct Collectible
    {
        glm::vec2 position;
        float radius = 22.0f;
        bool taken = false;
        std::string atlasName = "star";
    };

    struct Hazard
    {
        glm::vec2 position;
        glm::vec2 velocity;
        float radius = 26.0f;
        std::string atlasName = "orb";
    };

    struct Beacon
    {
        glm::vec2 position;
        glm::vec3 color;
        float radius;
        float intensity;
        std::string atlasName;
    };

    struct GameResources
    {
        bool loaded = false;
        std::shared_ptr<Shader> spriteShader;
        std::shared_ptr<Shader> lightShader;
        std::shared_ptr<Shader> screenShader;
        std::shared_ptr<Texture> playerSheet;
        std::shared_ptr<Texture> tileSheet;
        std::shared_ptr<Texture> particleTexture;
        std::shared_ptr<TextureAtlas> worldAtlas;
        std::shared_ptr<FontRenderer> debugFont;
        std::shared_ptr<AudioClip> jumpSfx;
        std::shared_ptr<AudioClip> pickupSfx;
        std::shared_ptr<AudioClip> hitSfx;
        std::shared_ptr<AudioClip> musicTrack;

        void load(Application &app)
        {
            if (loaded)
                return;

            spriteShader = app.assets().loadShader("sprite",
                                                   ASSETS_PATH "shaders/sprite.vert",
                                                   ASSETS_PATH "shaders/sprite.frag");
            lightShader = app.assets().loadShader("light",
                                                  ASSETS_PATH "shaders/light.vert",
                                                  ASSETS_PATH "shaders/light.frag");
            screenShader = app.assets().loadShader("screen",
                                                   ASSETS_PATH "shaders/screen.vert",
                                                   ASSETS_PATH "shaders/screen.frag");

            playerSheet = app.assets().loadTexture(ASSETS_PATH "generated/player_sheet.png",
                                                   TextureLoadOptions::pixelArt());
            tileSheet = app.assets().loadTexture(ASSETS_PATH "generated/tiles.png",
                                                 TextureLoadOptions::pixelArt());
            particleTexture = app.assets().loadTexture(ASSETS_PATH "textures/test.png",
                                                       TextureLoadOptions::smooth());

            TextureAtlasLoadOptions atlasOptions;
            atlasOptions.texture = TextureLoadOptions::pixelArt(false);
            atlasOptions.coordinateOrigin = AtlasCoordinateOrigin::TopLeft;
            atlasOptions.insetUVs = true;
            worldAtlas = app.assets().loadAtlas(ASSETS_PATH "atlas/world.atlas",
                                                ASSETS_PATH "atlas/world_atlas.png",
                                                atlasOptions);
            debugFont = app.assets().loadFont(ASSETS_PATH "fonts/roboto.ttf", 18);
            jumpSfx = app.assets().loadAudioClip("jump", ASSETS_PATH "audio/sfx/jump.wav");
            pickupSfx = app.assets().loadAudioClip("pickup", ASSETS_PATH "audio/sfx/pickup.wav");
            hitSfx = app.assets().loadAudioClip("hit", ASSETS_PATH "audio/sfx/hit.wav");
            musicTrack = app.assets().loadAudioClip("music", ASSETS_PATH "audio/music/theme.wav");

            app.audio().load(*jumpSfx);
            app.audio().load(*pickupSfx);
            app.audio().load(*hitSfx);
            app.audio().load(*musicTrack);
            loaded = true;
        }
    };

    GameResources &resources()
    {
        static GameResources res;
        return res;
    }

    float clampf(float v, float lo, float hi)
    {
        return std::max(lo, std::min(v, hi));
    }

    float lengthSquared(const glm::vec2 &v)
    {
        return v.x * v.x + v.y * v.y;
    }

    bool overlapsCircleAABB(const glm::vec2 &center, float radius,
                            const glm::vec2 &boxPos, const glm::vec2 &boxSize)
    {
        glm::vec2 nearest{
            clampf(center.x, boxPos.x, boxPos.x + boxSize.x),
            clampf(center.y, boxPos.y, boxPos.y + boxSize.y)};
        return lengthSquared(center - nearest) <= radius * radius;
    }

    void emitBurst(ParticleSystem &particles, const glm::vec2 &position,
                   const glm::vec4 &startColor, const glm::vec4 &endColor,
                   int count, float baseSpeed, float life, float size)
    {
        for (int i = 0; i < count; ++i)
        {
            float angle = (6.2831853f * (float)i) / (float)std::max(1, count);
            ParticleProps p;
            p.position = position;
            p.velocity = {std::cos(angle) * baseSpeed, std::sin(angle) * baseSpeed};
            p.velocityVariance = {26.0f, 26.0f};
            p.colorBegin = startColor;
            p.colorEnd = endColor;
            p.sizeBegin = size;
            p.sizeEnd = 2.0f;
            p.sizeVariance = 3.0f;
            p.lifeTime = life;
            particles.emit(p);
        }
    }

    class GameScene;
    class TitleScene;
    class PauseScene;
    class ResultScene;

    std::unique_ptr<Scene> makeTitleScene();
    std::unique_ptr<Scene> makeGameScene();
    std::unique_ptr<Scene> makePauseScene();
    std::unique_ptr<Scene> makeResultScene(bool won, int score, int totalStars, float timeSeconds);

    class TitleScene : public Scene
    {
    public:
        void onEnter() override
        {
            resources().load(App());
            m_t = 0.0f;
            App().audio().playMusic(*resources().musicTrack, 0.45f, 0.2f);
        }

        void onUpdate(float dt) override
        {
            m_t += dt;
            auto &ui = App().ui();
            UIRect card = ui.anchoredRect({420.0f, 260.0f}, UIAnchor::Center, {0.0f, -90.0f});
            ui.panel(card);
            UIRect content = ui.insetRect(card, {40.0f, 38.0f, 40.0f, 28.0f});
            auto stack = ui.stack(content, UIAxis::Vertical, 14.0f);
            ui.label("STARKEEP TRIAL", stack.next({content.size.x, 28.0f}).position,
                     {{0.95f, 0.98f, 1.0f, 1.0f}, 1.35f});
            ui.label("Collect every star, dodge the orbiters,", stack.next({content.size.x, 18.0f}).position,
                     {{0.82f, 0.89f, 0.96f, 1.0f}, 0.9f});
            ui.label("and reach the crystal.", stack.next({content.size.x, 18.0f}).position,
                     {{0.82f, 0.89f, 0.96f, 1.0f}, 0.9f});
            stack.next({content.size.x, 8.0f});
            ui.label("Move: A/D or arrows", stack.next({content.size.x, 18.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 0.95f}, 0.82f});
            ui.label("Jump: Space or Up", stack.next({content.size.x, 18.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 0.95f}, 0.82f});
            ui.label("Mouse click: spark burst", stack.next({content.size.x, 18.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 0.95f}, 0.82f});

            UIButtonStyle primary;
            primary.normalColor = {0.12f, 0.30f, 0.28f, 0.92f};
            primary.hoverColor = {0.18f, 0.42f, 0.38f, 0.96f};
            primary.activeColor = {0.24f, 0.52f, 0.46f, 0.98f};

            UIButtonStyle secondary;
            secondary.normalColor = {0.20f, 0.14f, 0.22f, 0.88f};
            secondary.hoverColor = {0.30f, 0.20f, 0.32f, 0.94f};
            secondary.activeColor = {0.40f, 0.25f, 0.40f, 0.98f};

            auto buttonStack = ui.stack(ui.anchoredRect({240.0f, 98.0f}, UIAnchor::Center, {0.0f, 130.0f}), UIAxis::Vertical, 10.0f);
            auto start = ui.button("title_start", "Start Run", buttonStack.next({240.0f, 46.0f}), primary);
            auto quit = ui.button("title_quit", "Quit", buttonStack.next({240.0f, 46.0f}), secondary);

            App().debug().setStat("showcase", "ECS + physics + lights + particles + audio");
            App().debug().setStat("post fx", "F1 vignette, F2 lighting, F3 saturation");

            if (start.clicked || Input::keyPressed(Key::Enter))
                manager->reset(makeGameScene());
            if (quit.clicked || Input::keyPressed(Key::Escape))
                App().quit();
        }

        void onDraw() override
        {
            float pulse = 0.1f + 0.04f * std::sin(m_t * 2.0f);
            RenderCommand::setClearColor({0.04f + pulse, 0.05f, 0.09f + pulse, 1.0f});
            RenderCommand::clear();
        }

    private:
        float m_t = 0.0f;
    };

    class PauseScene : public Scene
    {
    public:
        void onEnter() override
        {
            App().debug().print("Paused");
        }

        void onUpdate(float) override
        {
            auto &ui = App().ui();
            UIRect card = ui.anchoredRect({360.0f, 210.0f}, UIAnchor::Center);
            ui.panel(card);
            UIRect content = ui.insetRect(card, {34.0f, 34.0f, 34.0f, 26.0f});
            auto layout = ui.stack(content, UIAxis::Vertical, 12.0f);
            ui.label("PAUSED", layout.next({content.size.x, 26.0f}).position, {{1.0f, 1.0f, 1.0f, 1.0f}, 1.2f});
            layout.next({content.size.x, 10.0f});
            auto resume = ui.button("pause_resume", "Resume", layout.next({240.0f, 42.0f}));
            auto restart = ui.button("pause_restart", "Restart Run", layout.next({240.0f, 42.0f}));
            ui.label("Esc or P also resumes", layout.next({content.size.x, 16.0f}).position,
                     {{0.84f, 0.90f, 0.96f, 0.88f}, 0.75f});

            if (resume.clicked || Input::keyPressed(Key::P) || Input::keyPressed(Key::Escape))
                manager->pop();
            if (restart.clicked || Input::keyPressed(Key::R))
                manager->reset(makeGameScene());
        }
    };

    class ResultScene : public Scene
    {
    public:
        ResultScene(bool won, int score, int totalStars, float timeSeconds)
            : m_won(won), m_score(score), m_totalStars(totalStars), m_timeSeconds(timeSeconds) {}

        void onUpdate(float) override
        {
            auto &ui = App().ui();
            UIRect card = ui.anchoredRect({420.0f, 240.0f}, UIAnchor::Center);
            ui.panel(card);
            UIRect content = ui.insetRect(card, {38.0f, 34.0f, 38.0f, 24.0f});
            auto layout = ui.stack(content, UIAxis::Vertical, 14.0f);
            ui.label(m_won ? "RUN COMPLETE" : "RUN FAILED", layout.next({content.size.x, 26.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 1.0f}, 1.2f});
            ui.label(m_won ? "The crystal is lit." : "The orbiters got you.",
                     layout.next({content.size.x, 18.0f}).position, {{0.86f, 0.92f, 0.98f, 0.95f}, 0.86f});
            layout.next({content.size.x, 8.0f});
            auto again = ui.button("result_again", "Play Again", layout.next({240.0f, 42.0f}));
            auto title = ui.button("result_title", "Back To Title", layout.next({240.0f, 42.0f}));
            App().debug().setStat("score", m_score);
            App().debug().setStat("stars", std::to_string(m_totalStars) + "/" + std::to_string(m_totalStars));
            App().debug().setStat("time", m_timeSeconds);

            if (again.clicked || Input::keyPressed(Key::Enter))
                manager->reset(makeGameScene());
            if (title.clicked || Input::keyPressed(Key::Backspace))
                manager->reset(makeTitleScene());
        }

        void onDraw() override
        {
            RenderCommand::setClearColor(m_won
                                             ? glm::vec4{0.03f, 0.09f, 0.08f, 1.0f}
                                             : glm::vec4{0.12f, 0.03f, 0.06f, 1.0f});
            RenderCommand::clear();
        }

    private:
        bool m_won = false;
        int m_score = 0;
        int m_totalStars = 0;
        float m_timeSeconds = 0.0f;
    };

    class GameScene : public Scene
    {
    public:
        void onEnter() override
        {
            auto &res = resources();
            res.load(App());

            m_batch = std::make_unique<SpriteBatch>(*res.spriteShader);
            m_camera = std::make_unique<Camera2D>(0.0f, (float)SCREEN_W, 0.0f, (float)SCREEN_H);
            m_tilemap = std::make_unique<Tilemap>(MAP_W, MAP_H, TILE_SIZE, TILE_SIZE, 4);
            m_particles = std::make_unique<ParticleSystem>(3000);
            m_post = std::make_unique<PostProcessor>(SCREEN_W, SCREEN_H, *res.screenShader);
            m_lights = std::make_unique<LightRenderer>(SCREEN_W, SCREEN_H, *res.lightShader);

            m_tilemap->setAtlas(res.tileSheet);
            m_fx.exposure = 1.05f;
            m_fx.vignette = 0.35f;
            m_fx.saturation = 1.1f;
            m_fx.useLights = true;
            m_physics.gravity = {0.0f, -1200.0f};
            m_world = World{};
            m_stars.clear();
            m_hazards.clear();
            m_beacons.clear();
            m_collisionsThisFrame = 0;
            m_totalCollisions = 0;
            m_score = 0;
            m_time = 0.0f;
            m_totalStars = 0;
            m_finished = false;
            m_resultQueued = false;
            m_tookHit = false;
            m_groundFlash = 0.0f;
            m_recentCollision.clear();

            buildLevel();
            buildSignals();
            bindEvents();
            App().audio().playMusic(*resources().musicTrack, 0.38f, 0.2f);
        }

        void onExit() override
        {
            if (m_collisionListener != INVALID_LISTENER)
                EventBus::off<CollisionEvent>(m_collisionListener);
            if (m_soundListener != INVALID_LISTENER)
                EventBus::off<SoundFinishedEvent>(m_soundListener);
            if (m_resizeListener != INVALID_LISTENER)
                EventBus::off<WindowResizedEvent>(m_resizeListener);
            m_collisionListener = INVALID_LISTENER;
            m_soundListener = INVALID_LISTENER;
            m_resizeListener = INVALID_LISTENER;
            m_scoreChanged.disconnectAll();
            m_runEnded.disconnectAll();
        }

        void onUpdate(float dt) override
        {
            m_time += dt;
            m_groundFlash = std::max(0.0f, m_groundFlash - dt * 2.0f);
            m_collisionsThisFrame = 0;

            if (Input::keyPressed(Key::Escape) || Input::keyPressed(Key::P))
            {
                manager->push(makePauseScene());
                return;
            }

            if (Input::keyPressed(Key::F1))
                m_fx.vignette = (m_fx.vignette > 0.0f) ? 0.0f : 0.35f;
            if (Input::keyPressed(Key::F2))
                m_fx.useLights = !m_fx.useLights;
            if (Input::keyPressed(Key::F3))
                m_fx.saturation = (m_fx.saturation > 1.2f) ? 0.75f : 1.45f;

            updatePlayerInput();
            updateActors(dt);
            m_physics.update(m_world, dt);
            processTriggers();
            updateCamera(dt);
            m_particles->update(dt);
            updateDebug();
        }

        void onDraw() override
        {
            auto &res = resources();
            auto &playerTransform = m_world.get<Transform>(m_player);

            m_post->beginScene();
            RenderCommand::setClearColor({0.04f, 0.06f + 0.02f * m_groundFlash, 0.10f, 1.0f});
            RenderCommand::clear();

            m_batch->begin(m_camera->viewProjection());
            drawParallax();
            m_tilemap->draw(*m_batch);
            drawProps();
            drawCollectibles();
            drawHazards();
            drawActors();
            m_particles->draw(*m_batch, *res.particleTexture);
            m_batch->flush();
            m_post->endScene();

            m_lights->begin(m_camera->viewProjection());
            m_lights->submitLight({playerTransform.position + glm::vec2{24.0f, 26.0f},
                                   {0.95f, 0.88f, 0.6f},
                                   280.0f,
                                   1.15f});
            for (const auto &b : m_beacons)
            {
                m_lights->submitLight({b.position, b.color, b.radius, b.intensity});
            }
            for (const auto &star : m_stars)
            {
                if (!star.taken)
                    m_lights->submitLight({star.position + glm::vec2{8.0f, 8.0f},
                                           {1.0f, 0.92f, 0.45f},
                                           120.0f,
                                           0.55f});
            }
            m_lights->end();

            m_post->composite(m_lights->lightMapID(), m_fx);
        }

    private:
        void buildSignals()
        {
            m_scoreChanged.connect([this](int score, int starsLeft)
                                   {
                                       m_fx.exposure = 1.0f + 0.015f * (float)(m_totalStars - starsLeft);
                                       if (starsLeft == 0)
                                           App().debug().print("All stars gathered. Head to the crystal.");
                                       else if (score > 0)
                                           App().debug().print("Star secured."); });

            m_runEnded.connect([this](bool won)
                               {
                                   if (m_resultQueued)
                                       return;
                                   m_resultQueued = true;
                                   manager->replace(makeResultScene(won, m_score, m_totalStars, m_time)); });
        }

        void bindEvents()
        {
            m_collisionListener = EventBus::on<CollisionEvent>(
                [this](const CollisionEvent &e)
                {
                    if (e.entityA == m_player || e.entityB == m_player)
                    {
                        ++m_collisionsThisFrame;
                        ++m_totalCollisions;
                        m_recentCollision = "player collision";
                        if (e.normal.y > 0.5f)
                            m_groundFlash = 0.3f;
                    }
                });

            m_soundListener = EventBus::on<SoundFinishedEvent>(
                [this](const SoundFinishedEvent &e)
                {
                    if (e.name == "pickup")
                        App().debug().print("Chime faded."); });

            m_resizeListener = EventBus::on<WindowResizedEvent>(
                [this](const WindowResizedEvent &e)
                {
                    App().debug().print("Window resized to " + std::to_string(e.width) + "x" + std::to_string(e.height));
                });
        }

        void buildLevel()
        {
            for (int x = 0; x < MAP_W; ++x)
            {
                setSolidTile(x, 0, 1);
                if (x % 3 == 0)
                    setSolidTile(x, 1, 0);
            }

            for (int x = 4; x <= 7; ++x)
                setSolidTile(x, 3, x == 5 ? 2 : 0);
            for (int x = 10; x <= 14; ++x)
                setSolidTile(x, 5, x == 12 ? 2 : 1);
            for (int x = 17; x <= 21; ++x)
                setSolidTile(x, 7, x == 19 ? 2 : 0);
            for (int x = 23; x <= 26; ++x)
                setSolidTile(x, 9, x == 26 ? 3 : 1);

            spawnPlayer({120.0f, 160.0f});
            spawnWalker({520.0f, 320.0f}, 110.0f, 380.0f, 620.0f, {1.0f, 0.55f, 0.55f, 1.0f});
            spawnWalker({1180.0f, 448.0f}, 140.0f, 980.0f, 1320.0f, {1.0f, 0.70f, 0.45f, 1.0f});
            spawnWalker({1500.0f, 576.0f}, 160.0f, 1420.0f, 1680.0f, {0.85f, 0.55f, 1.0f, 1.0f});

            spawnStar({344.0f, 272.0f});
            spawnStar({792.0f, 400.0f});
            spawnStar({1216.0f, 528.0f});
            spawnStar({1600.0f, 656.0f});

            m_goalMin = {1664.0f, 640.0f};
            m_goalMax = m_goalMin + glm::vec2{72.0f, 96.0f};

            m_hazards.push_back({{610.0f, 392.0f}, {85.0f, 0.0f}, 28.0f, "orb"});
            m_hazards.push_back({{1340.0f, 560.0f}, {-110.0f, 0.0f}, 28.0f, "orb"});

            m_beacons.push_back({{338.0f, 312.0f}, {0.9f, 0.8f, 0.4f}, 170.0f, 0.45f, "lantern"});
            m_beacons.push_back({{1212.0f, 560.0f}, {0.4f, 0.8f, 1.0f}, 220.0f, 0.55f, "lantern"});
            m_beacons.push_back({m_goalMin + glm::vec2{32.0f, 50.0f}, {0.45f, 1.0f, 0.9f}, 260.0f, 0.85f, "crystal"});
        }

        void setSolidTile(int x, int y, int tileID)
        {
            m_tilemap->setTile(x, y, tileID);
            spawnStatic((float)(x * TILE_SIZE), (float)(y * TILE_SIZE),
                        (float)TILE_SIZE, (float)TILE_SIZE);
        }

        void spawnPlayer(const glm::vec2 &position)
        {
            auto &res = resources();
            m_player = m_world.create();
            m_world.add<Transform>(m_player, Transform{position});
            m_world.add<Sprite>(m_player, Sprite{res.playerSheet, {64.0f, 64.0f}, {1.0f, 1.0f, 1.0f, 1.0f}});
            m_world.add<RigidBody>(m_player, RigidBody{});
            m_world.add<BoxCollider>(m_player, BoxCollider{{42.0f, 56.0f}, {11.0f, 4.0f}});
            m_world.add<Tag>(m_player, Tag{"player"});

            AnimatorComponent anim;
            anim.animator.addAnimation(Animator::makeAnimation("idle", 64, 16, 16, 16, 0, 4, 0.22f, true));
            anim.animator.addAnimation(Animator::makeAnimation("run", 64, 16, 16, 16, 0, 4, 0.10f, true));
            anim.animator.play("idle");
            m_world.add<AnimatorComponent>(m_player, anim);
        }

        void spawnWalker(const glm::vec2 &position, float speed, float minX, float maxX, const glm::vec4 &color)
        {
            auto &res = resources();
            EntityID enemy = m_world.create();
            m_world.add<Transform>(enemy, Transform{position});
            m_world.add<Sprite>(enemy, Sprite{res.playerSheet, {56.0f, 56.0f}, color});
            RigidBody rb;
            rb.velocity.x = speed;
            m_world.add<RigidBody>(enemy, rb);
            m_world.add<BoxCollider>(enemy, BoxCollider{{40.0f, 46.0f}, {8.0f, 6.0f}});
            m_world.add<Tag>(enemy, Tag{"walker"});

            AnimatorComponent anim;
            anim.animator.addAnimation(Animator::makeAnimation("run", 64, 16, 16, 16, 0, 4, 0.12f, true));
            anim.animator.play("run");
            anim.animator.flipX = speed < 0.0f;
            m_world.add<AnimatorComponent>(enemy, anim);

            m_patrols[enemy] = {minX, maxX, speed};
        }

        void spawnStatic(float x, float y, float w, float h)
        {
            EntityID e = m_world.create();
            m_world.add<Transform>(e, Transform{{x, y}});
            RigidBody rb;
            rb.isStatic = true;
            m_world.add<RigidBody>(e, rb);
            m_world.add<BoxCollider>(e, BoxCollider{{w, h}});
            m_world.add<Tag>(e, Tag{"solid"});
        }

        void spawnStar(const glm::vec2 &position)
        {
            m_stars.push_back({position, 20.0f, false, "star"});
            ++m_totalStars;
        }

        void updatePlayerInput()
        {
            auto &t = m_world.get<Transform>(m_player);
            auto &rb = m_world.get<RigidBody>(m_player);
            auto &anim = m_world.get<AnimatorComponent>(m_player).animator;

            float move = 0.0f;
            if (Input::keyDown(Key::A) || Input::keyDown(Key::Left))
                move -= 1.0f;
            if (Input::keyDown(Key::D) || Input::keyDown(Key::Right))
                move += 1.0f;

            rb.velocity.x = move * 280.0f;
            if (move != 0.0f)
                anim.flipX = move < 0.0f;

            if ((Input::keyPressed(Key::Space) || Input::keyPressed(Key::Up)) && rb.onGround)
            {
                rb.velocity.y = 620.0f;
                App().audio().play(*resources().jumpSfx, 0.7f, 1.0f);
                emitBurst(*m_particles, t.position + glm::vec2{22.0f, 8.0f},
                          {0.95f, 0.90f, 0.65f, 0.9f}, {0.90f, 0.50f, 0.2f, 0.0f},
                          10, 90.0f, 0.45f, 10.0f);
            }

            if (Input::mousePressed(MouseButton::Left))
            {
                glm::vec2 logicalMouse = App().window().windowToLogical(Input::mousePos());
                glm::vec2 worldMouse = m_camera->screenToWorld(logicalMouse, App().window().logicalSize(), true);
                emitBurst(*m_particles, worldMouse,
                          {0.50f, 0.90f, 1.0f, 1.0f}, {0.40f, 0.20f, 1.0f, 0.0f},
                          18, 140.0f, 0.65f, 14.0f);
                App().debug().print("Spark burst");
            }

            anim.play(std::abs(move) > 0.1f ? "run" : "idle");
        }

        void updateActors(float dt)
        {
            m_world.view<Transform, RigidBody, Tag>().each(
                [&](EntityID e, Transform &t, RigidBody &rb, Tag &tag)
                {
                    if (tag.name == "walker")
                    {
                        auto patrol = m_patrols.find(e);
                        if (patrol != m_patrols.end())
                        {
                            if (t.position.x < patrol->second.minX)
                                patrol->second.speed = std::abs(patrol->second.speed);
                            if (t.position.x > patrol->second.maxX)
                                patrol->second.speed = -std::abs(patrol->second.speed);
                            rb.velocity.x = patrol->second.speed;

                            auto &anim = m_world.get<AnimatorComponent>(e).animator;
                            anim.flipX = patrol->second.speed < 0.0f;
                        }
                    }
                });

            m_world.view<AnimatorComponent>().each(
                [&](EntityID, AnimatorComponent &anim)
                {
                    anim.animator.update(dt);
                });

            for (auto &hazard : m_hazards)
            {
                hazard.position += hazard.velocity * dt;
                if (hazard.position.x < 560.0f || hazard.position.x > WORLD_W - 180.0f)
                    hazard.velocity.x *= -1.0f;
            }
        }

        void processTriggers()
        {
            auto &playerTransform = m_world.get<Transform>(m_player);
            auto &playerBody = m_world.get<RigidBody>(m_player);
            auto &playerCollider = m_world.get<BoxCollider>(m_player);
            glm::vec2 playerBoxPos = playerTransform.position + playerCollider.offset;

            for (auto &star : m_stars)
            {
                if (star.taken)
                    continue;
                if (overlapsCircleAABB(star.position, star.radius, playerBoxPos, playerCollider.size))
                {
                    star.taken = true;
                    m_score += 100;
                    App().audio().play(*resources().pickupSfx, 0.8f, 1.0f);
                    emitBurst(*m_particles, star.position + glm::vec2{8.0f, 8.0f},
                              {1.0f, 0.95f, 0.55f, 1.0f}, {1.0f, 0.45f, 0.2f, 0.0f},
                              20, 160.0f, 0.7f, 18.0f);
                    m_scoreChanged.emit(m_score, starsRemaining());
                }
            }

            for (auto &hazard : m_hazards)
            {
                if (overlapsCircleAABB(hazard.position, hazard.radius, playerBoxPos, playerCollider.size))
                {
                    if (!m_tookHit)
                    {
                        m_tookHit = true;
                        App().audio().play(*resources().hitSfx, 0.8f, 0.95f);
                        emitBurst(*m_particles, playerTransform.position + glm::vec2{24.0f, 24.0f},
                                  {1.0f, 0.35f, 0.55f, 1.0f}, {0.3f, 0.0f, 0.1f, 0.0f},
                                  28, 190.0f, 0.9f, 16.0f);
                        m_runEnded.emit(false);
                    }
                    break;
                }
            }

            if (!m_finished &&
                playerTransform.position.x + playerCollider.offset.x + playerCollider.size.x >= m_goalMin.x &&
                playerTransform.position.y + playerCollider.offset.y + playerCollider.size.y >= m_goalMin.y &&
                starsRemaining() == 0)
            {
                m_finished = true;
                emitBurst(*m_particles, m_goalMin + glm::vec2{36.0f, 44.0f},
                          {0.45f, 1.0f, 0.9f, 1.0f}, {0.1f, 0.6f, 1.0f, 0.0f},
                          32, 180.0f, 1.1f, 18.0f);
                m_runEnded.emit(true);
            }

            if (playerTransform.position.y < -180.0f && !m_tookHit)
            {
                m_tookHit = true;
                m_runEnded.emit(false);
            }
        }

        void updateCamera(float dt)
        {
            auto &playerTransform = m_world.get<Transform>(m_player);
            glm::vec2 target = playerTransform.position - glm::vec2{SCREEN_W * 0.35f, SCREEN_H * 0.35f};
            target.x = clampf(target.x, 0.0f, WORLD_W - (float)SCREEN_W);
            target.y = clampf(target.y, 0.0f, 420.0f);
            m_camera->setPosition(glm::mix(m_camera->position(), target, 7.0f * dt));
        }

        void updateDebug()
        {
            std::ostringstream starsText;
            starsText << (m_totalStars - starsRemaining()) << "/" << m_totalStars;
            auto &ui = App().ui();
            UIRect hud = ui.anchoredRect({240.0f, 118.0f}, UIAnchor::TopLeft, {20.0f, 20.0f});
            ui.panel(hud, {{0.04f, 0.08f, 0.14f, 0.72f}, {0.9f, 0.95f, 1.0f, 0.10f}, 2.0f});
            auto layout = ui.stack(ui.insetRect(hud, {16.0f, 18.0f, 16.0f, 14.0f}), UIAxis::Vertical, 8.0f);
            ui.label("Run HUD", layout.next({200.0f, 18.0f}).position, {{0.95f, 0.98f, 1.0f, 1.0f}, 0.95f});
            ui.label("Stars " + starsText.str(), layout.next({200.0f, 18.0f}).position, {{1.0f, 0.93f, 0.60f, 1.0f}, 0.82f});
            ui.label("Score " + std::to_string(m_score), layout.next({200.0f, 18.0f}).position, {{0.85f, 1.0f, 0.92f, 1.0f}, 0.82f});
            ui.label("Time " + [&]() { std::ostringstream ss; ss << std::fixed << std::setprecision(1) << m_time; return ss.str(); }(),
                     layout.next({200.0f, 18.0f}).position, {{0.84f, 0.90f, 0.96f, 1.0f}, 0.82f});

            App().debug().print("Enter the crystal with every star.");
            App().debug().setStat("score", m_score);
            App().debug().setStat("stars", starsText.str());
            App().debug().setStat("time", m_time);
            App().debug().setStat("entities", (int)m_world.entityCount());
            App().debug().setStat("particles", (int)m_particles->activeCount());
            App().debug().setStat("lights", m_fx.useLights ? 1 : 0);
            App().debug().setStat("collisions/frame", m_collisionsThisFrame);
            App().debug().setStat("collisions/total", m_totalCollisions);
        }

        void drawParallax()
        {
            auto &atlas = *resources().worldAtlas;
            float cameraX = m_camera->position().x;
            for (int i = 0; i < 7; ++i)
            {
                float x = std::fmod((float)(i * 260) - cameraX * 0.18f, WORLD_W);
                if (x < -120.0f)
                    x += WORLD_W;
                atlas.draw(*m_batch, "cloud", x, 560.0f + 18.0f * std::sin((cameraX + i * 15.0f) * 0.01f), 96.0f, 52.0f,
                           {0.7f, 0.82f, 1.0f, 0.28f});
            }
        }

        void drawProps()
        {
            auto &atlas = *resources().worldAtlas;
            for (const auto &b : m_beacons)
            {
                atlas.draw(*m_batch, b.atlasName, b.position.x - 18.0f, b.position.y - 18.0f, 36.0f, 36.0f);
            }

            atlas.draw(*m_batch, "banner", m_goalMin.x - 14.0f, m_goalMin.y + 8.0f, 42.0f, 84.0f,
                       {1.0f, 1.0f, 1.0f, 0.85f});
            atlas.draw(*m_batch, "crystal", m_goalMin.x + 20.0f, m_goalMin.y + 16.0f, 40.0f, 56.0f);
            atlas.draw(*m_batch, "shrub", 180.0f, 88.0f, 40.0f, 30.0f);
            atlas.draw(*m_batch, "shrub", 1480.0f, 600.0f, 40.0f, 30.0f);
        }

        void drawCollectibles()
        {
            auto &atlas = *resources().worldAtlas;
            float bob = std::sin(m_time * 4.0f) * 6.0f;
            for (const auto &star : m_stars)
            {
                if (star.taken)
                    continue;
                atlas.draw(*m_batch, star.atlasName, star.position.x, star.position.y + bob, 28.0f, 28.0f);
            }
        }

        void drawHazards()
        {
            auto &atlas = *resources().worldAtlas;
            for (const auto &hazard : m_hazards)
            {
                atlas.draw(*m_batch, hazard.atlasName, hazard.position.x - hazard.radius, hazard.position.y - hazard.radius,
                           hazard.radius * 2.0f, hazard.radius * 2.0f);
            }
        }

        void drawActors()
        {
            m_world.view<Transform, Sprite, Tag, AnimatorComponent>().each(
                [&](EntityID, Transform &t, Sprite &sprite, Tag &tag, AnimatorComponent &anim)
                {
                    glm::vec4 color = sprite.color;
                    if (tag.name == "player" && starsRemaining() == 0)
                        color = {0.85f, 1.0f, 0.8f, 1.0f};
                    anim.animator.draw(*m_batch, *sprite.texture,
                                       t.position.x, t.position.y,
                                       sprite.size.x, sprite.size.y, color);
                });
        }

        int starsRemaining() const
        {
            int remaining = 0;
            for (const auto &star : m_stars)
            {
                if (!star.taken)
                    ++remaining;
            }
            return remaining;
        }

        struct PatrolRange
        {
            float minX;
            float maxX;
            float speed;
        };

        World m_world;
        PhysicsSystem m_physics;
        EntityID m_player = NULL_ENTITY;
        PostFX m_fx;

        std::unique_ptr<SpriteBatch> m_batch;
        std::unique_ptr<Camera2D> m_camera;
        std::unique_ptr<Tilemap> m_tilemap;
        std::unique_ptr<ParticleSystem> m_particles;
        std::unique_ptr<PostProcessor> m_post;
        std::unique_ptr<LightRenderer> m_lights;

        std::unordered_map<EntityID, PatrolRange> m_patrols;
        std::vector<Collectible> m_stars;
        std::vector<Hazard> m_hazards;
        std::vector<Beacon> m_beacons;
        glm::vec2 m_goalMin{0.0f};
        glm::vec2 m_goalMax{0.0f};

        Signal<int, int> m_scoreChanged;
        Signal<bool> m_runEnded;
        ListenerID m_collisionListener = INVALID_LISTENER;
        ListenerID m_soundListener = INVALID_LISTENER;
        ListenerID m_resizeListener = INVALID_LISTENER;

        int m_score = 0;
        int m_totalStars = 0;
        int m_collisionsThisFrame = 0;
        int m_totalCollisions = 0;
        float m_time = 0.0f;
        float m_groundFlash = 0.0f;
        bool m_finished = false;
        bool m_resultQueued = false;
        bool m_tookHit = false;
        std::string m_recentCollision;
    };

    std::unique_ptr<Scene> makeTitleScene()
    {
        return std::make_unique<TitleScene>();
    }

    std::unique_ptr<Scene> makeGameScene()
    {
        return std::make_unique<GameScene>();
    }

    std::unique_ptr<Scene> makePauseScene()
    {
        return std::make_unique<PauseScene>();
    }

    std::unique_ptr<Scene> makeResultScene(bool won, int score, int totalStars, float timeSeconds)
    {
        return std::make_unique<ResultScene>(won, score, totalStars, timeSeconds);
    }
} // namespace

class NebulaApp : public Application
{
public:
    NebulaApp() : Application({"Nebula Showcase", SCREEN_W, SCREEN_H}) {}

    void onInit() override
    {
        resources().load(*this);
        auto fontShader = assets().loadShader("font",
                                              ASSETS_PATH "shaders/font.vert",
                                              ASSETS_PATH "shaders/font.frag");

        m_debugBatch = std::make_unique<SpriteBatch>(*fontShader);
        debug().init(resources().debugFont, window().logicalWidth(), window().logicalHeight());
        ui().init(resources().debugFont, window().logicalWidth(), window().logicalHeight());
        scenes().reset(makeTitleScene());
    }
};

int main()
{
    NebulaApp app;
    app.run();
}
