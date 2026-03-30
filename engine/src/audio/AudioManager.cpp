#include "nebula/audio/AudioManager.h"
#include "nebula/events/EventBus.h"
#include "nebula/events/Events.h"
#include <miniaudio.h>
#include <stdexcept>
#include <iostream>

namespace nebula
{

    AudioManager::AudioManager()
    {
        m_engine = new ma_engine;
        if (ma_engine_init(nullptr, m_engine) != MA_SUCCESS)
            throw std::runtime_error("Failed to init miniaudio engine");
    }

    AudioManager::~AudioManager()
    {
        if (m_engine)
        {
            for (auto &sound : m_sfx)
            {
                if (sound.handle)
                    ma_sound_uninit(sound.handle.get());
            }
            m_sfx.clear();

            for (auto &[_, music] : m_music)
            {
                if (music)
                    ma_sound_uninit(music.get());
            }
            m_music.clear();
            ma_engine_uninit(m_engine);
            delete m_engine;
        }
    }

    void AudioManager::load(const std::string &name, const std::string &path)
    {
        m_paths[name] = path;
    }

    void AudioManager::load(const AudioClip &clip)
    {
        load(clip.name, clip.path);
    }

    void AudioManager::unload(const std::string &name)
    {
        m_paths.erase(name);
        if (m_currentMusic == name)
            m_currentMusic.clear();

        auto musicIt = m_music.find(name);
        if (musicIt != m_music.end())
        {
            if (musicIt->second)
                ma_sound_uninit(musicIt->second.get());
            m_music.erase(musicIt);
        }
    }

    void AudioManager::play(const std::string &name, float volume, float pitch)
    {
        auto it = m_paths.find(name);
        if (it == m_paths.end())
        {
            std::cerr << "[Audio] Sound not loaded: " << name << "\n";
            return;
        }

        auto sound = std::make_unique<ma_sound>();
        if (ma_sound_init_from_file(m_engine, it->second.c_str(),
                                    MA_SOUND_FLAG_ASYNC, nullptr, nullptr,
                                    sound.get()) != MA_SUCCESS)
        {
            std::cerr << "[Audio] Failed to play: " << name << "\n";
            return;
        }

        float effectiveVol = volume * m_sfxVol * m_masterVol;
        ma_sound_set_volume(sound.get(), effectiveVol);
        ma_sound_set_pitch(sound.get(), pitch);
        ma_sound_start(sound.get());

        m_sfx.push_back({std::move(sound), name});
    }

    void AudioManager::play(const AudioClip &clip, float volume, float pitch)
    {
        load(clip);
        play(clip.name, volume, pitch);
    }

    void AudioManager::playMusic(const std::string &name,
                                 float volume, float fadeDuration)
    {
        // stop current music
        if (!m_currentMusic.empty())
        {
            auto it = m_music.find(m_currentMusic);
            if (it != m_music.end())
            {
                if (fadeDuration > 0)
                    ma_sound_set_fade_in_milliseconds(
                        it->second.get(), 1.0f, 0.0f,
                        (ma_uint64)(fadeDuration * 1000));
                else
                    ma_sound_stop(it->second.get());

                ma_sound_uninit(it->second.get());
                m_music.erase(it);
            }
        }

        auto it = m_paths.find(name);
        if (it == m_paths.end())
        {
            std::cerr << "[Audio] Music not loaded: " << name << "\n";
            return;
        }

        auto sound = std::make_unique<ma_sound>();
        if (ma_sound_init_from_file(m_engine, it->second.c_str(),
                                    MA_SOUND_FLAG_ASYNC, nullptr, nullptr,
                                    sound.get()) != MA_SUCCESS)
        {
            std::cerr << "[Audio] Failed to load music: " << name << "\n";
            return;
        }

        float effectiveVol = volume * m_musicVol * m_masterVol;
        ma_sound_set_looping(sound.get(), MA_TRUE);
        ma_sound_set_volume(sound.get(), 0.0f);
        ma_sound_start(sound.get());

        if (fadeDuration > 0)
            ma_sound_set_fade_in_milliseconds(sound.get(), 0.0f, effectiveVol,
                                              (ma_uint64)(fadeDuration * 1000));
        else
            ma_sound_set_volume(sound.get(), effectiveVol);

        m_music[name] = std::move(sound);
        m_currentMusic = name;
    }

    void AudioManager::playMusic(const AudioClip &clip, float volume, float fadeDuration)
    {
        load(clip);
        playMusic(clip.name, volume, fadeDuration);
    }

    void AudioManager::stopMusic(float fadeDuration)
    {
        if (m_currentMusic.empty())
            return;
        auto it = m_music.find(m_currentMusic);
        if (it != m_music.end())
        {
            if (fadeDuration > 0)
                ma_sound_set_fade_in_milliseconds(it->second.get(), -1, 0,
                                                  (ma_uint64)(fadeDuration * 1000));
            else
                ma_sound_stop(it->second.get());
        }
        m_currentMusic.clear();
    }

    void AudioManager::pauseMusic()
    {
        if (m_currentMusic.empty())
            return;
        auto it = m_music.find(m_currentMusic);
        if (it != m_music.end())
            ma_sound_stop(it->second.get());
    }

    void AudioManager::resumeMusic()
    {
        if (m_currentMusic.empty())
            return;
        auto it = m_music.find(m_currentMusic);
        if (it != m_music.end())
            ma_sound_start(it->second.get());
    }

    void AudioManager::setMasterVolume(float v)
    {
        m_masterVol = v;
        ma_engine_set_volume(m_engine, v);
    }

    void AudioManager::setSfxVolume(float v) { m_sfxVol = v; }
    void AudioManager::setMusicVolume(float v)
    {
        m_musicVol = v;
        if (!m_currentMusic.empty())
        {
            auto it = m_music.find(m_currentMusic);
            if (it != m_music.end())
                ma_sound_set_volume(it->second.get(), v * m_masterVol);
        }
    }

    bool AudioManager::isPlaying(const std::string &name) const
    {
        auto it = m_music.find(name);
        if (it == m_music.end())
            return false;
        return ma_sound_is_playing(it->second.get());
    }

    bool AudioManager::isMusicPlaying() const
    {
        return !m_currentMusic.empty() && isPlaying(m_currentMusic);
    }

    void AudioManager::update()
    {
        for (auto it = m_sfx.begin(); it != m_sfx.end();)
        {
            if (!ma_sound_is_playing(it->handle.get()))
            {
                EventBus::defer(SoundFinishedEvent{it->name});
                ma_sound_uninit(it->handle.get());
                it = m_sfx.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // remove stopped music tracks to free memory
        for (auto it = m_music.begin(); it != m_music.end();)
        {
            if (it->first != m_currentMusic &&
                !ma_sound_is_playing(it->second.get()))
            {
                ma_sound_uninit(it->second.get());
                it = m_music.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

} // namespace nebula
