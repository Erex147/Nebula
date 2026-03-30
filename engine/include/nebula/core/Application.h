#pragma once
#include <memory>
#include "nebula/core/Window.h"
#include "nebula/core/Input.h"
#include "nebula/assets/AssetManager.h"
#include "nebula/scene/SceneManager.h"
#include "nebula/debug/DebugOverlay.h"
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/audio/AudioManager.h"
#include "nebula/ui/UILayer.h"

namespace nebula
{

    class Application
    {
    public:
        explicit Application(const WindowConfig &cfg);
        virtual ~Application() = default;

        void run();
        void quit() { m_running = false; }

        Window &window() { return *m_window; }
        const Window &window() const { return *m_window; }
        AssetManager &assets() { return m_assets; }
        SceneManager &scenes() { return m_scenes; }
        DebugOverlay &debug() { return m_debug; }
        AudioManager &audio() { return m_audio; }
        UILayer &ui() { return m_ui; }

        // Override to push your first scene
        virtual void onInit() {}
        virtual void onShutdown() {}

    protected:
        std::unique_ptr<Window> m_window;
        std::unique_ptr<SpriteBatch> m_debugBatch;
        AssetManager m_assets;
        SceneManager m_scenes;
        DebugOverlay m_debug;
        AudioManager m_audio;
        UILayer m_ui;
        bool m_running = true;
    };

    // global accessor — set once in Application constructor
    Application &App();

} // namespace nebula
