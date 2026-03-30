#pragma once
#include <memory>
#include "nebula/core/Window.h"
#include "nebula/assets/AssetManager.h"
#include "nebula/core/Input.h"

namespace nebula {

class Application {
public:
    explicit Application(const WindowConfig& cfg);
    virtual ~Application() = default;

    void run();
    void quit() { m_running = false; }

    virtual void onInit()           {}
    virtual void onUpdate(float dt) {}
    virtual void onDraw()           {}
    virtual void onShutdown()       {}

    Window&       window() { return *m_window; }
    AssetManager& assets() { return m_assets; }

protected:
    std::unique_ptr<Window> m_window;
    AssetManager            m_assets;
    bool                    m_running = true;
};

} // namespace nebula