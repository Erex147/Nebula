#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "nebula/renderer/SpriteBatch.h"
#include "nebula/renderer/Texture.h"

namespace nebula
{

    struct AnimationFrame
    {
        // UV coords for this frame
        float u0, v0, u1, v1;
        float duration; // seconds this frame displays
    };

    struct Animation
    {
        std::string name;
        std::vector<AnimationFrame> frames;
        bool loop = true;
        bool flipX = false;
    };

    class Animator
    {
    public:
        // build from a spritesheet:
        //   sheetW/sheetH = full texture dimensions
        //   frameW/frameH = size of one frame
        //   row           = which row in the sheet
        //   count         = how many frames
        static Animation makeAnimation(const std::string &name,
                                       int sheetW, int sheetH,
                                       int frameW, int frameH,
                                       int row, int count,
                                       float frameDuration = 0.1f,
                                       bool loop = true);

        void addAnimation(const Animation &anim);
        void play(const std::string &name, bool forceRestart = false);

        void update(float dt);

        // draw current frame
        void draw(SpriteBatch &batch, const Texture &texture,
                  float x, float y, float w, float h,
                  const glm::vec4 &color = glm::vec4(1.0f));

        bool isPlaying(const std::string &name) const;
        bool isFinished() const; // true on last frame of non-looping anim
        int currentFrame() const { return m_frameIdx; }
        const std::string &currentAnimation() const { return m_current; }

        bool flipX = false;
        bool flipY = false;

    private:
        std::unordered_map<std::string, Animation> m_anims;
        std::string m_current;
        int m_frameIdx = 0;
        float m_timer = 0;
        bool m_finished = false;
    };

} // namespace nebula