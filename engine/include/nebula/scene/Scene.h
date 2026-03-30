#pragma once

namespace nebula
{

    class SceneManager;

    class Scene
    {
    public:
        virtual ~Scene() = default;

        // called once when the scene first becomes active
        virtual void onEnter() {}
        // called once when the scene is popped off the stack
        virtual void onExit() {}
        // called when another scene is pushed ON TOP of this one
        virtual void onPause() {}
        // called when the scene on top of this one is popped
        virtual void onResume() {}

        virtual void onUpdate(float dt) {}
        virtual void onDraw() {}

        SceneManager *manager = nullptr;
    };

} // namespace nebula