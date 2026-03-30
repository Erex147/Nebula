#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "nebula/audio/AudioClip.h"

// forward declare so miniaudio.h stays out of public headers
typedef struct ma_engine ma_engine;
typedef struct ma_sound ma_sound;

namespace nebula
{

    struct SoundInstance
    {
        std::unique_ptr<ma_sound> handle;
        std::string name;
    };

    class AudioManager
    {
    public:
        AudioManager();
        ~AudioManager();

        // load a sound into memory (call once, then play by name)
        void load(const std::string &name, const std::string &path);
        void load(const AudioClip &clip);
        void unload(const std::string &name);

        // one-shot sound effect
        void play(const std::string &name,
                  float volume = 1.0f,
                  float pitch = 1.0f);
        void play(const AudioClip &clip,
                  float volume = 1.0f,
                  float pitch = 1.0f);

        // looping music track — only one plays at a time
        void playMusic(const std::string &name, float volume = 1.0f,
                       float fadeDuration = 0.5f);
        void playMusic(const AudioClip &clip, float volume = 1.0f,
                       float fadeDuration = 0.5f);
        void stopMusic(float fadeDuration = 0.5f);
        void pauseMusic();
        void resumeMusic();

        // global volume
        void setMasterVolume(float v);
        void setSfxVolume(float v);
        void setMusicVolume(float v);
        float masterVolume() const { return m_masterVol; }
        float sfxVolume() const { return m_sfxVol; }
        float musicVolume() const { return m_musicVol; }

        bool isPlaying(const std::string &name) const;
        bool isMusicPlaying() const;

        // call once per frame
        void update();

    private:
        ma_engine *m_engine = nullptr;
        std::unordered_map<std::string, std::string> m_paths; // name -> file path
        std::vector<SoundInstance> m_sfx;
        std::unordered_map<std::string,
                           std::unique_ptr<ma_sound>>
            m_music;
        std::string m_currentMusic;

        float m_masterVol = 1.0f;
        float m_sfxVol = 1.0f;
        float m_musicVol = 1.0f;
    };

} // namespace nebula
