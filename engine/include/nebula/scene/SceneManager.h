#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "nebula/scene/Scene.h"

namespace nebula
{

    class SceneManager
    {
    public:
        // push a new scene onto the stack
        void push(std::unique_ptr<Scene> scene);

        // pop the top scene
        void pop();

        // replace the top scene (pop + push in one)
        void replace(std::unique_ptr<Scene> scene);

        // clear all scenes and push a fresh one
        void reset(std::unique_ptr<Scene> scene);

        void update(float dt);
        void draw();

        Scene *top() const;
        bool empty() const { return m_stack.empty(); }

        // apply any pending push/pop operations
        // called once per frame after update+draw
        void applyPending();

    private:
        enum class Action
        {
            Push,
            Pop,
            Replace,
            Reset
        };

        struct Command
        {
            Action action;
            std::unique_ptr<Scene> scene;
        };

        std::vector<std::unique_ptr<Scene>> m_stack;
        std::vector<Command> m_pending;
    };

} // namespace nebula