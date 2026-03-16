#include "SoundSystemSwitch.h"
#include "../../client/sound/Sound.h"
#include <cstring>

SoundSystemSwitch::SoundSystemSwitch()
:   _initialized(false),
    _enabled(true),
    _outputFreq(22050),
    _outputFormat(AUDIO_S16SYS),
    _outputChannels(2)
{
    if (Mix_OpenAudio(_outputFreq, _outputFormat, _outputChannels, 1024) == 0) {
        Mix_AllocateChannels(16);
        _initialized = true;
    }
}

SoundSystemSwitch::~SoundSystemSwitch()
{

    for (std::map<const char*, Mix_Chunk*>::iterator it = _chunkCache.begin(); it != _chunkCache.end(); ++it) {
        if (it->second) {
            if (it->second->abuf) {
                SDL_free(it->second->abuf);
                it->second->abuf = NULL;
            }
            Mix_FreeChunk(it->second);
        }
    }
    _chunkCache.clear();

    if (_initialized) {
        Mix_CloseAudio();
    }
}

Mix_Chunk* SoundSystemSwitch::getOrCreateChunk(const SoundDesc& desc)
{
    if (!desc.frames) return NULL;

    std::map<const char*, Mix_Chunk*>::iterator it = _chunkCache.find(desc.frames);
    if (it != _chunkCache.end()) return it->second;

    SDL_AudioFormat srcFormat = (desc.byteWidth == 2) ? AUDIO_S16SYS : AUDIO_S8;

    SDL_AudioCVT cvt;
    int ret = SDL_BuildAudioCVT(&cvt, srcFormat, desc.channels, desc.frameRate,
                                 _outputFormat, _outputChannels, _outputFreq);

    if (ret < 0) return NULL;

    int bufLen = desc.size * ((ret > 0) ? cvt.len_mult : 1);
    Uint8* buf = (Uint8*)SDL_malloc(bufLen);
    if (!buf) return NULL;

    memcpy(buf, desc.frames, desc.size);

    Uint32 finalLen = desc.size;
    if (ret > 0) {
        cvt.len = desc.size;
        cvt.buf = buf;
        SDL_ConvertAudio(&cvt);
        finalLen = cvt.len_cvt;
    }

    Mix_Chunk* chunk = Mix_QuickLoad_RAW(buf, finalLen);
    if (chunk) {
        _chunkCache[desc.frames] = chunk;
    } else {
        SDL_free(buf);
    }
    return chunk;
}

void SoundSystemSwitch::playAt(const SoundDesc& desc, float x, float y, float z, float volume, float pitch)
{
    if (!_initialized || !_enabled) return;
    if (!desc.isValid()) return;

    Mix_Chunk* chunk = getOrCreateChunk(desc);
    if (!chunk) return;

    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel >= 0) {
        int vol = (int)(volume * MIX_MAX_VOLUME);
        if (vol > MIX_MAX_VOLUME) vol = MIX_MAX_VOLUME;
        if (vol < 0) vol = 0;
        Mix_Volume(channel, vol);
    }
}
