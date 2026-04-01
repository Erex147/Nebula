#include "nebula/assets.h"
#include "nebula/audio.h"
#include "nebula/core.h"
#include "nebula/math.h"
#include "nebula/renderer.h"
#include "nebula/scene.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace nebula;

namespace
{
    constexpr int SCREEN_W = 1280;
    constexpr int SCREEN_H = 720;
    constexpr float WORLD_W = (float)SCREEN_W;
    constexpr float WORLD_H = (float)SCREEN_H;

    struct GameResources
    {
        bool loaded = false;
        std::shared_ptr<Shader> spriteShader;
        std::shared_ptr<FontRenderer> debugFont;
        std::shared_ptr<TextureAtlas> worldAtlas;
        std::shared_ptr<AudioClip> shootSfx;
        std::shared_ptr<AudioClip> hitSfx;
        std::shared_ptr<AudioClip> winSfx;
        std::shared_ptr<AudioClip> musicTrack;

        void load(Application &app)
        {
            if (loaded)
                return;

            spriteShader = app.assets().loadShader("sprite",
                                                   ASSETS_PATH "shaders/sprite.vert",
                                                   ASSETS_PATH "shaders/sprite.frag");

            TextureAtlasLoadOptions atlasOptions;
            atlasOptions.texture = TextureLoadOptions::pixelArt(false);
            atlasOptions.coordinateOrigin = AtlasCoordinateOrigin::TopLeft;
            atlasOptions.insetUVs = true;

            debugFont = app.assets().loadFont(ASSETS_PATH "fonts/roboto.ttf", 18);
            worldAtlas = app.assets().loadAtlas(ASSETS_PATH "atlas/world.atlas",
                                                ASSETS_PATH "atlas/world_atlas.png",
                                                atlasOptions);
            shootSfx = app.assets().loadAudioClip("shoot", ASSETS_PATH "audio/sfx/jump.wav");
            hitSfx = app.assets().loadAudioClip("hit", ASSETS_PATH "audio/sfx/hit.wav");
            winSfx = app.assets().loadAudioClip("win", ASSETS_PATH "audio/sfx/pickup.wav");
            musicTrack = app.assets().loadAudioClip("music", ASSETS_PATH "audio/music/theme.wav");

            app.audio().load(*shootSfx);
            app.audio().load(*hitSfx);
            app.audio().load(*winSfx);
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

    bool overlapsAABB(const glm::vec2 &aPos, const glm::vec2 &aSize,
                      const glm::vec2 &bPos, const glm::vec2 &bSize)
    {
        return aPos.x < bPos.x + bSize.x &&
               aPos.x + aSize.x > bPos.x &&
               aPos.y < bPos.y + bSize.y &&
               aPos.y + aSize.y > bPos.y;
    }

    std::string formatTime(float seconds)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << seconds << "s";
        return ss.str();
    }

    struct Bullet
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{6.0f, 18.0f};
        float speed = 0.0f;
        bool fromPlayer = false;
        bool alive = true;
    };

    struct Invader
    {
        glm::vec2 localPosition{0.0f};
        glm::vec2 size{40.0f, 40.0f};
        glm::vec4 tint{1.0f};
        std::string spriteName;
        bool alive = true;
        int scoreValue = 0;
    };

    struct ShieldBlock
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{16.0f, 16.0f};
        int health = 3;
        bool alive = true;
    };

    struct Star
    {
        glm::vec2 position{0.0f};
        float radius = 2.0f;
        float drift = 0.0f;
        glm::vec4 color{1.0f};
    };

    class TitleScene;
    class PauseScene;
    class ResultScene;
    class GameScene;

    std::unique_ptr<Scene> makeTitleScene();
    std::unique_ptr<Scene> makePauseScene();
    std::unique_ptr<Scene> makeGameScene();
    std::unique_ptr<Scene> makeResultScene(int score, int waveReached, float timeSeconds);

    class TitleScene : public Scene
    {
    public:
        void onEnter() override
        {
            resources().load(App());
            m_time = 0.0f;
            App().audio().playMusic(*resources().musicTrack, 0.35f, 0.3f);
        }

        void onUpdate(float dt) override
        {
            m_time += dt;
            auto &ui = App().ui();

            UIRect card = ui.anchoredRect({540.0f, 340.0f}, UIAnchor::Center, {0.0f, -36.0f});
            ui.panel(card, {{0.03f, 0.05f, 0.10f, 0.84f}, {0.82f, 0.92f, 1.0f, 0.14f}, 2.0f});
            auto layout = ui.stack(ui.insetRect(card, {38.0f, 34.0f, 38.0f, 24.0f}), UIAxis::Vertical, 12.0f);

            ui.label("NEBULA INVADERS", layout.next({420.0f, 30.0f}).position,
                     {{0.95f, 0.98f, 1.0f, 1.0f}, 1.4f});
            ui.label("Hold the line, break the fleet, and survive forever.",
                     layout.next({450.0f, 18.0f}).position,
                     {{0.82f, 0.90f, 0.98f, 1.0f}, 0.92f});
            layout.next({450.0f, 8.0f});
            ui.label("Move: A / D or arrows", layout.next({420.0f, 18.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 0.95f}, 0.84f});
            ui.label("Shoot: Space", layout.next({420.0f, 18.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 0.95f}, 0.84f});
            ui.label("Pause: Escape or P", layout.next({420.0f, 18.0f}).position,
                     {{1.0f, 1.0f, 1.0f, 0.95f}, 0.84f});
            ui.label("Each cleared fleet starts a faster and meaner new wave.",
                     layout.next({450.0f, 18.0f}).position,
                     {{0.96f, 0.86f, 0.68f, 1.0f}, 0.82f});

            UIButtonStyle primary;
            primary.normalColor = {0.08f, 0.28f, 0.26f, 0.96f};
            primary.hoverColor = {0.14f, 0.40f, 0.36f, 1.0f};
            primary.activeColor = {0.22f, 0.52f, 0.44f, 1.0f};

            UIButtonStyle secondary;
            secondary.normalColor = {0.18f, 0.12f, 0.26f, 0.88f};
            secondary.hoverColor = {0.26f, 0.20f, 0.34f, 0.94f};
            secondary.activeColor = {0.34f, 0.28f, 0.44f, 1.0f};

            auto buttons = ui.stack(ui.anchoredRect({240.0f, 102.0f}, UIAnchor::Center, {0.0f, 132.0f}),
                                    UIAxis::Vertical, 10.0f);
            auto start = ui.button("title_start", "Start", buttons.next({240.0f, 46.0f}), primary);
            auto quit = ui.button("title_quit", "Quit", buttons.next({240.0f, 46.0f}), secondary);

            App().debug().setStat("sample", "Nebula Invaders");
            App().debug().setStat("mode", "title");

            if (start.clicked || Input::keyPressed(Key::Enter) || Input::keyPressed(Key::Space))
                manager->reset(makeGameScene());
            if (quit.clicked || Input::keyPressed(Key::Escape))
                App().quit();
        }

        void onDraw() override
        {
            const float pulse = 0.03f * std::sin(m_time * 1.2f);
            RenderCommand::setClearColor({0.03f, 0.04f + pulse, 0.09f + pulse * 1.4f, 1.0f});
            RenderCommand::clear();
        }

    private:
        float m_time = 0.0f;
    };

    class PauseScene : public Scene
    {
    public:
        void onUpdate(float) override
        {
            auto &ui = App().ui();
            UIRect card = ui.anchoredRect({340.0f, 220.0f}, UIAnchor::Center);
            ui.panel(card, {{0.04f, 0.06f, 0.10f, 0.88f}, {0.9f, 0.95f, 1.0f, 0.16f}, 2.0f});
            auto layout = ui.stack(ui.insetRect(card, {30.0f, 28.0f, 30.0f, 24.0f}), UIAxis::Vertical, 14.0f);
            ui.label("PAUSED", layout.next({260.0f, 26.0f}).position, {{0.96f, 0.98f, 1.0f, 1.0f}, 1.2f});
            ui.label("The fleet is holding position.", layout.next({260.0f, 18.0f}).position,
                     {{0.82f, 0.88f, 0.96f, 1.0f}, 0.9f});
            layout.next({260.0f, 10.0f});

            auto buttons = ui.stack(ui.anchoredRect({220.0f, 102.0f}, UIAnchor::Center, {0.0f, 54.0f}),
                                    UIAxis::Vertical, 10.0f);
            auto resume = ui.button("pause_resume", "Resume", buttons.next({220.0f, 46.0f}));
            auto restart = ui.button("pause_restart", "Restart", buttons.next({220.0f, 46.0f}));

            if (resume.clicked || Input::keyPressed(Key::Escape) || Input::keyPressed(Key::P))
                manager->pop();
            if (restart.clicked)
                manager->reset(makeGameScene());
        }

        void onDraw() override {}
    };

    class ResultScene : public Scene
    {
    public:
        ResultScene(int score, int waveReached, float timeSeconds)
            : m_score(score), m_waveReached(waveReached), m_timeSeconds(timeSeconds)
        {
        }

        void onUpdate(float) override
        {
            auto &ui = App().ui();
            UIRect card = ui.anchoredRect({420.0f, 256.0f}, UIAnchor::Center, {0.0f, -48.0f});
            ui.panel(card, {{0.04f, 0.07f, 0.12f, 0.84f}, {0.9f, 0.95f, 1.0f, 0.16f}, 2.0f});
            auto layout = ui.stack(ui.insetRect(card, {34.0f, 30.0f, 34.0f, 24.0f}), UIAxis::Vertical, 12.0f);
            ui.label("BASE OVERRUN", layout.next({320.0f, 28.0f}).position,
                     UILabelStyle{{1.0f, 0.78f, 0.78f, 1.0f}, 1.15f});
            ui.label("The invaders finally broke through.",
                     layout.next({320.0f, 18.0f}).position,
                     {{0.84f, 0.90f, 0.98f, 1.0f}, 0.9f});
            layout.next({320.0f, 10.0f});
            ui.label("Score " + std::to_string(m_score), layout.next({320.0f, 18.0f}).position,
                     {{1.0f, 0.93f, 0.60f, 1.0f}, 0.86f});
            ui.label("Wave " + std::to_string(m_waveReached), layout.next({320.0f, 18.0f}).position,
                     {{0.84f, 1.0f, 0.90f, 1.0f}, 0.86f});
            ui.label("Time " + formatTime(m_timeSeconds), layout.next({320.0f, 18.0f}).position,
                     {{0.84f, 0.90f, 0.98f, 1.0f}, 0.86f});

            auto buttons = ui.stack(ui.anchoredRect({240.0f, 102.0f}, UIAnchor::Center, {0.0f, 136.0f}),
                                    UIAxis::Vertical, 10.0f);
            auto again = ui.button("result_again", "Play Again", buttons.next({240.0f, 46.0f}));
            auto title = ui.button("result_title", "Back To Title", buttons.next({240.0f, 46.0f}));

            if (again.clicked || Input::keyPressed(Key::Enter) || Input::keyPressed(Key::Space))
                manager->reset(makeGameScene());
            if (title.clicked || Input::keyPressed(Key::Escape))
                manager->reset(makeTitleScene());
        }

        void onDraw() override
        {
            RenderCommand::setClearColor({0.11f, 0.05f, 0.08f, 1.0f});
            RenderCommand::clear();
        }

    private:
        int m_score = 0;
        int m_waveReached = 1;
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
            m_camera = std::make_unique<Camera2D>(0.0f, WORLD_W, 0.0f, WORLD_H);

            m_rng.seed(42u);
            m_time = 0.0f;
            m_score = 0;
            m_lives = 3;
            m_wave = 1;
            m_playerPosition = {WORLD_W * 0.5f - m_playerSize.x * 0.5f, 56.0f};
            m_playerBulletCooldown = 0.0f;
            m_enemyFireCooldown = 0.9f;
            m_resultQueued = false;
            m_recentEvent = "Fleet incoming";

            buildStars();
            startWave(true);
            App().audio().playMusic(*resources().musicTrack, 0.28f, 0.25f);
        }

        void onUpdate(float dt) override
        {
            m_time += dt;
            m_playerBulletCooldown = std::max(0.0f, m_playerBulletCooldown - dt);
            m_enemyFireCooldown = std::max(0.0f, m_enemyFireCooldown - dt);

            if (Input::keyPressed(Key::Escape) || Input::keyPressed(Key::P))
            {
                manager->push(makePauseScene());
                return;
            }

            updateStars(dt);
            updatePlayer(dt);
            updateFleet(dt);
            updateBullets(dt);
            maybeFireEnemyShot();
            updateHudAndDebug();
            evaluateOutcome();
        }

        void onDraw() override
        {
            RenderCommand::setClearColor({0.02f, 0.03f, 0.08f, 1.0f});
            RenderCommand::clear();

            m_batch->begin(m_camera->viewProjection());
            drawStars();
            drawBaseLine();
            drawShields();
            drawInvaders();
            drawPlayer();
            drawBullets();
            m_batch->flush();
        }

    private:
        void buildStars()
        {
            m_stars.clear();
            std::uniform_real_distribution<float> xDist(0.0f, WORLD_W);
            std::uniform_real_distribution<float> yDist(0.0f, WORLD_H);
            std::uniform_real_distribution<float> driftDist(6.0f, 22.0f);
            std::uniform_real_distribution<float> tone(0.55f, 1.0f);

            for (int i = 0; i < 48; ++i)
            {
                const float c = tone(m_rng);
                m_stars.push_back({{xDist(m_rng), yDist(m_rng)},
                                   (i % 3 == 0) ? 3.0f : 2.0f,
                                   driftDist(m_rng),
                                   {0.55f * c, 0.72f * c, c, 0.95f}});
            }
        }

        void buildInvaders()
        {
            m_invaders.clear();

            const glm::vec4 rowTints[5] = {
                {1.0f, 0.82f, 0.58f, 1.0f},
                {1.0f, 0.74f, 0.66f, 1.0f},
                {0.88f, 1.0f, 1.0f, 1.0f},
                {0.76f, 0.95f, 1.0f, 1.0f},
                {0.84f, 1.0f, 0.88f, 1.0f}};
            const char *rowSprites[5] = {"ember", "spark", "wisp", "crystal", "lantern"};
            const int rowScores[5] = {40, 30, 20, 20, 10};

            for (int row = 0; row < 5; ++row)
            {
                for (int col = 0; col < 9; ++col)
                {
                    Invader invader;
                    invader.localPosition = {(float)(col * 84), (float)(-row * 58)};
                    invader.spriteName = rowSprites[row];
                    invader.tint = rowTints[row];
                    invader.scoreValue = rowScores[row];
                    m_invaders.push_back(invader);
                }
            }
        }

        void startWave(bool resetShields)
        {
            m_bullets.clear();
            if (resetShields)
                buildShields();

            buildInvaders();
            m_fleetDirection = (m_wave % 2 == 0) ? -1.0f : 1.0f;
            m_fleetSpeed = 48.0f + (float)(m_wave - 1) * 9.0f;
            m_fleetOrigin = {204.0f, 540.0f};
            m_enemyFireCooldown = std::max(0.28f, 0.95f - (float)(m_wave - 1) * 0.05f);
            m_lowestInvaderY = WORLD_H;
            m_recentEvent = m_wave == 1 ? "Fleet incoming" : "Wave " + std::to_string(m_wave) + " started";
        }

        void buildShields()
        {
            m_shields.clear();
            const float shieldY = 145.0f;
            const float shieldStarts[4] = {190.0f, 430.0f, 670.0f, 910.0f};

            for (float startX : shieldStarts)
            {
                for (int row = 0; row < 4; ++row)
                {
                    for (int col = 0; col < 6; ++col)
                    {
                        if ((row == 3 && (col == 0 || col == 5)) ||
                            (row == 2 && (col == 2 || col == 3)))
                            continue;

                        ShieldBlock block;
                        block.position = {startX + col * 18.0f, shieldY + row * 18.0f};
                        m_shields.push_back(block);
                    }
                }
            }
        }

        void updateStars(float dt)
        {
            for (auto &star : m_stars)
            {
                star.position.y -= star.drift * dt;
                if (star.position.y < -8.0f)
                    star.position.y = WORLD_H + 8.0f;
            }
        }

        void updatePlayer(float dt)
        {
            float move = 0.0f;
            if (Input::keyDown(Key::A) || Input::keyDown(Key::Left))
                move -= 1.0f;
            if (Input::keyDown(Key::D) || Input::keyDown(Key::Right))
                move += 1.0f;

            m_playerPosition.x += move * m_playerSpeed * dt;
            m_playerPosition.x = clampf(m_playerPosition.x, 24.0f, WORLD_W - m_playerSize.x - 24.0f);

            if ((Input::keyPressed(Key::Space) || Input::keyPressed(Key::Up)) && m_playerBulletCooldown <= 0.0f)
            {
                Bullet bullet;
                bullet.position = m_playerPosition + glm::vec2{m_playerSize.x * 0.5f - 3.0f, m_playerSize.y};
                bullet.speed = 540.0f;
                bullet.fromPlayer = true;
                m_bullets.push_back(bullet);
                m_playerBulletCooldown = 0.22f;
                m_recentEvent = "Player fired";
                App().audio().play(*resources().shootSfx, 0.45f, 1.2f);
            }
        }

        void updateFleet(float dt)
        {
            float minX = WORLD_W;
            float maxX = 0.0f;
            float lowestY = WORLD_H;
            bool anyAlive = false;

            for (const auto &invader : m_invaders)
            {
                if (!invader.alive)
                    continue;

                anyAlive = true;
                const glm::vec2 pos = invaderWorldPosition(invader);
                minX = std::min(minX, pos.x);
                maxX = std::max(maxX, pos.x + invader.size.x);
                lowestY = std::min(lowestY, pos.y);
            }

            if (!anyAlive)
                return;

            m_fleetOrigin.x += m_fleetDirection * m_fleetSpeed * dt;

            if ((m_fleetDirection > 0.0f && maxX >= WORLD_W - 42.0f) ||
                (m_fleetDirection < 0.0f && minX <= 42.0f))
            {
                m_fleetDirection *= -1.0f;
                m_fleetOrigin.y -= 24.0f;
                m_fleetSpeed = std::min(m_fleetSpeed + 8.0f, 120.0f);
                m_recentEvent = "Fleet descended";
            }

            m_lowestInvaderY = lowestY;
        }

        void updateBullets(float dt)
        {
            for (auto &bullet : m_bullets)
            {
                if (!bullet.alive)
                    continue;

                bullet.position.y += bullet.speed * dt * (bullet.fromPlayer ? 1.0f : -1.0f);

                if (bullet.position.y > WORLD_H + 24.0f || bullet.position.y < -30.0f)
                    bullet.alive = false;
            }

            for (auto &bullet : m_bullets)
            {
                if (!bullet.alive)
                    continue;

                for (auto &shield : m_shields)
                {
                    if (!shield.alive)
                        continue;
                    if (overlapsAABB(bullet.position, bullet.size, shield.position, shield.size))
                    {
                        bullet.alive = false;
                        shield.health -= 1;
                        if (shield.health <= 0)
                            shield.alive = false;
                        break;
                    }
                }
            }

            for (auto &bullet : m_bullets)
            {
                if (!bullet.alive || !bullet.fromPlayer)
                    continue;

                for (auto &invader : m_invaders)
                {
                    if (!invader.alive)
                        continue;

                    const glm::vec2 invaderPos = invaderWorldPosition(invader);
                    if (overlapsAABB(bullet.position, bullet.size, invaderPos, invader.size))
                    {
                        bullet.alive = false;
                        invader.alive = false;
                        m_score += invader.scoreValue;
                        m_fleetSpeed = std::min(m_fleetSpeed + 2.5f, 150.0f);
                        m_recentEvent = "Invader destroyed";
                        App().audio().play(*resources().hitSfx, 0.5f, 1.1f);
                        break;
                    }
                }
            }

            for (auto &bullet : m_bullets)
            {
                if (!bullet.alive || bullet.fromPlayer)
                    continue;

                if (overlapsAABB(bullet.position, bullet.size, m_playerPosition, m_playerSize))
                {
                    bullet.alive = false;
                    m_lives -= 1;
                    m_recentEvent = "Player hit";
                    App().audio().play(*resources().hitSfx, 0.62f, 0.9f);
                }
            }

            m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
                                           [](const Bullet &bullet)
                                           { return !bullet.alive; }),
                            m_bullets.end());
        }

        void maybeFireEnemyShot()
        {
            if (m_enemyFireCooldown > 0.0f)
                return;

            std::vector<int> firingCandidates;
            for (int col = 0; col < 9; ++col)
            {
                for (int row = 4; row >= 0; --row)
                {
                    const int idx = row * 9 + col;
                    if (idx < (int)m_invaders.size() && m_invaders[idx].alive)
                    {
                        firingCandidates.push_back(idx);
                        break;
                    }
                }
            }

            if (firingCandidates.empty())
                return;

            std::uniform_int_distribution<int> pick(0, (int)firingCandidates.size() - 1);
            const Invader &shooter = m_invaders[firingCandidates[pick(m_rng)]];

            Bullet bullet;
            bullet.position = invaderWorldPosition(shooter) + glm::vec2{shooter.size.x * 0.5f - 3.0f, -6.0f};
            bullet.speed = 280.0f + (150.0f - std::min(m_fleetSpeed, 150.0f)) * 0.35f;
            bullet.fromPlayer = false;
            bullet.size = {6.0f, 16.0f};
            m_bullets.push_back(bullet);

            const float minDelay = std::max(0.18f, 0.45f - (float)(m_wave - 1) * 0.02f);
            const float maxDelay = std::max(minDelay + 0.12f, 1.15f - (float)(m_wave - 1) * 0.035f);
            std::uniform_real_distribution<float> delay(minDelay, maxDelay);
            m_enemyFireCooldown = delay(m_rng);
        }

        void evaluateOutcome()
        {
            if (m_resultQueued)
                return;

            const bool cleared = std::none_of(m_invaders.begin(), m_invaders.end(),
                                              [](const Invader &invader)
                                              { return invader.alive; });
            if (cleared)
            {
                ++m_wave;
                App().audio().play(*resources().winSfx, 0.72f, 1.0f);
                startWave(true);
                return;
            }

            if (m_lives <= 0 || m_lowestInvaderY <= 118.0f)
            {
                m_resultQueued = true;
                manager->replace(makeResultScene(m_score, m_wave, m_time));
            }
        }

        void updateHudAndDebug()
        {
            auto &ui = App().ui();
            UIRect hud = ui.anchoredRect({278.0f, 132.0f}, UIAnchor::TopLeft, {20.0f, 18.0f});
            ui.panel(hud, {{0.03f, 0.07f, 0.12f, 0.72f}, {0.9f, 0.95f, 1.0f, 0.10f}, 2.0f});
            auto layout = ui.stack(ui.insetRect(hud, {16.0f, 16.0f, 16.0f, 14.0f}), UIAxis::Vertical, 8.0f);

            ui.label("Invaders HUD", layout.next({220.0f, 18.0f}).position,
                     {{0.96f, 0.98f, 1.0f, 1.0f}, 0.95f});
            ui.label("Score " + std::to_string(m_score), layout.next({220.0f, 18.0f}).position,
                     {{1.0f, 0.92f, 0.58f, 1.0f}, 0.84f});
            ui.label("Wave " + std::to_string(m_wave), layout.next({220.0f, 18.0f}).position,
                     {{0.84f, 1.0f, 0.90f, 1.0f}, 0.84f});
            ui.label("Lives " + std::to_string(m_lives), layout.next({220.0f, 18.0f}).position,
                     {{0.84f, 1.0f, 0.90f, 1.0f}, 0.84f});
            ui.label("Time " + formatTime(m_time), layout.next({220.0f, 18.0f}).position,
                     {{0.82f, 0.90f, 0.98f, 1.0f}, 0.84f});
            ui.label("Objective: Survive the endless waves",
                     layout.next({220.0f, 18.0f}).position, UILabelStyle{{0.96f, 0.86f, 0.68f, 1.0f}, 0.78f});

            App().debug().print("Nebula Invaders running.");
            App().debug().setStat("score", m_score);
            App().debug().setStat("wave", m_wave);
            App().debug().setStat("lives", m_lives);
            App().debug().setStat("fleet speed", m_fleetSpeed);
            App().debug().setStat("bullets", (int)m_bullets.size());
            App().debug().setStat("invaders", aliveInvaderCount());
            App().debug().setStat("event", m_recentEvent);
        }

        void drawStars()
        {
            for (const auto &star : m_stars)
                m_batch->drawColorQuad(star.position.x, star.position.y, star.radius, star.radius, star.color);
        }

        void drawBaseLine()
        {
            m_batch->drawColorQuad(52.0f, 96.0f, WORLD_W - 104.0f, 4.0f, {0.20f, 0.86f, 0.68f, 0.85f});
        }

        void drawPlayer()
        {
            auto &atlas = *resources().worldAtlas;
            m_batch->drawColorQuad(m_playerPosition.x, m_playerPosition.y, m_playerSize.x, m_playerSize.y,
                                   {0.10f, 0.18f, 0.24f, 1.0f});
            atlas.draw(*m_batch, "banner", m_playerPosition.x + 10.0f, m_playerPosition.y + 4.0f,
                       28.0f, 34.0f, {1.0f, 0.92f, 0.74f, 1.0f});
            atlas.draw(*m_batch, "crystal", m_playerPosition.x + 22.0f, m_playerPosition.y + 18.0f,
                       22.0f, 18.0f, {0.72f, 1.0f, 0.92f, 1.0f});
        }

        void drawInvaders()
        {
            auto &atlas = *resources().worldAtlas;
            for (const auto &invader : m_invaders)
            {
                if (!invader.alive)
                    continue;

                const glm::vec2 pos = invaderWorldPosition(invader);
                m_batch->drawColorQuad(pos.x - 4.0f, pos.y - 2.0f, invader.size.x + 8.0f, invader.size.y + 4.0f,
                                       {0.06f, 0.10f, 0.18f, 0.85f});
                atlas.draw(*m_batch, invader.spriteName, pos.x, pos.y,
                           invader.size.x, invader.size.y, invader.tint);
            }
        }

        void drawShields()
        {
            for (const auto &shield : m_shields)
            {
                if (!shield.alive)
                    continue;

                glm::vec4 color = {0.28f, 0.78f, 0.60f, 1.0f};
                if (shield.health == 2)
                    color = {0.72f, 0.82f, 0.42f, 1.0f};
                else if (shield.health == 1)
                    color = {0.90f, 0.58f, 0.38f, 1.0f};
                m_batch->drawColorQuad(shield.position.x, shield.position.y, shield.size.x, shield.size.y, color);
            }
        }

        void drawBullets()
        {
            for (const auto &bullet : m_bullets)
            {
                const glm::vec4 color = bullet.fromPlayer ? glm::vec4{0.92f, 0.96f, 1.0f, 1.0f}
                                                          : glm::vec4{1.0f, 0.56f, 0.64f, 1.0f};
                m_batch->drawColorQuad(bullet.position.x, bullet.position.y, bullet.size.x, bullet.size.y, color);
            }
        }

        glm::vec2 invaderWorldPosition(const Invader &invader) const
        {
            return m_fleetOrigin + invader.localPosition;
        }

        int aliveInvaderCount() const
        {
            return (int)std::count_if(m_invaders.begin(), m_invaders.end(),
                                      [](const Invader &invader)
                                      { return invader.alive; });
        }

        std::unique_ptr<SpriteBatch> m_batch;
        std::unique_ptr<Camera2D> m_camera;

        std::vector<Star> m_stars;
        std::vector<Invader> m_invaders;
        std::vector<ShieldBlock> m_shields;
        std::vector<Bullet> m_bullets;

        std::mt19937 m_rng;
        glm::vec2 m_playerPosition{0.0f};
        glm::vec2 m_playerSize{62.0f, 34.0f};
        glm::vec2 m_fleetOrigin{0.0f};
        float m_playerSpeed = 420.0f;
        float m_playerBulletCooldown = 0.0f;
        float m_enemyFireCooldown = 0.0f;
        float m_fleetDirection = 1.0f;
        float m_fleetSpeed = 52.0f;
        float m_lowestInvaderY = WORLD_H;
        float m_time = 0.0f;
        int m_score = 0;
        int m_lives = 3;
        int m_wave = 1;
        bool m_resultQueued = false;
        std::string m_recentEvent = "Fleet incoming";
    };

    std::unique_ptr<Scene> makeTitleScene()
    {
        return std::make_unique<TitleScene>();
    }

    std::unique_ptr<Scene> makePauseScene()
    {
        return std::make_unique<PauseScene>();
    }

    std::unique_ptr<Scene> makeGameScene()
    {
        return std::make_unique<GameScene>();
    }

    std::unique_ptr<Scene> makeResultScene(int score, int waveReached, float timeSeconds)
    {
        return std::make_unique<ResultScene>(score, waveReached, timeSeconds);
    }
} // namespace

class NebulaApp : public Application
{
public:
    NebulaApp() : Application({"Nebula Invaders", SCREEN_W, SCREEN_H}) {}

    void onInit() override
    {
        resources().load(*this);
        installDebugOverlay();
        installDeveloperTools();

        auto fontShader = assets().loadShader("font",
                                              ASSETS_PATH "shaders/font.vert",
                                              ASSETS_PATH "shaders/font.frag");

        m_debugBatch = std::make_unique<SpriteBatch>(*fontShader);
        if (hasDebugOverlay())
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
