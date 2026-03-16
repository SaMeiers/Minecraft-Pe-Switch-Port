#ifndef SOUNDSYSTEM_SWITCH_H__
#define SOUNDSYSTEM_SWITCH_H__

#include "SoundSystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <map>

class SoundSystemSwitch : public SoundSystem
{
public:
    SoundSystemSwitch();
    virtual ~SoundSystemSwitch();

    bool isAvailable() override { return _initialized; }
    void enable(bool status) override { _enabled = status; }
    void setListenerPos(float x, float y, float z) override {}
    void setListenerAngle(float deg) override {}
    void load(const std::string& name) override {}
    void play(const std::string& name) override {}
    void pause(const std::string& name) override {}
    void stop(const std::string& name) override {}
    void playAt(const SoundDesc& desc, float x, float y, float z, float volume, float pitch) override;

    void update() {}
    void setVolume(float volume) {}

private:
    Mix_Chunk* getOrCreateChunk(const SoundDesc& desc);

    bool _initialized;
    bool _enabled;
    int _outputFreq;
    Uint16 _outputFormat;
    int _outputChannels;
    std::map<const char*, Mix_Chunk*> _chunkCache;
};

#endif 
