#include "nebula/scene/SceneManager.h"
#include <stdexcept>

namespace nebula
{

    void SceneManager::push(std::unique_ptr<Scene> scene)
    {
        scene->manager = this;
        m_pending.push_back({Action::Push, std::move(scene)});
    }

    void SceneManager::pop()
    {
        m_pending.push_back({Action::Pop, nullptr});
    }

    void SceneManager::replace(std::unique_ptr<Scene> scene)
    {
        scene->manager = this;
        m_pending.push_back({Action::Replace, std::move(scene)});
    }

    void SceneManager::reset(std::unique_ptr<Scene> scene)
    {
        scene->manager = this;
        m_pending.push_back({Action::Reset, std::move(scene)});
    }

    void SceneManager::update(float dt)
    {
        if (!m_stack.empty())
            m_stack.back()->onUpdate(dt);
    }

    void SceneManager::draw()
    {
        // draw all scenes bottom to top so pause menus overlay game world
        for (auto &s : m_stack)
            s->onDraw();
    }

    Scene *SceneManager::top() const
    {
        return m_stack.empty() ? nullptr : m_stack.back().get();
    }

    void SceneManager::applyPending()
    {
        for (auto &cmd : m_pending)
        {
            switch (cmd.action)
            {
            case Action::Push:
                if (!m_stack.empty())
                    m_stack.back()->onPause();
                m_stack.push_back(std::move(cmd.scene));
                m_stack.back()->onEnter();
                break;

            case Action::Pop:
                if (!m_stack.empty())
                {
                    m_stack.back()->onExit();
                    m_stack.pop_back();
                }
                if (!m_stack.empty())
                    m_stack.back()->onResume();
                break;

            case Action::Replace:
                if (!m_stack.empty())
                {
                    m_stack.back()->onExit();
                    m_stack.pop_back();
                }
                m_stack.push_back(std::move(cmd.scene));
                m_stack.back()->onEnter();
                break;

            case Action::Reset:
                while (!m_stack.empty())
                {
                    m_stack.back()->onExit();
                    m_stack.pop_back();
                }
                m_stack.push_back(std::move(cmd.scene));
                m_stack.back()->onEnter();
                break;
            }
        }
        m_pending.clear();
    }

} // namespace nebula