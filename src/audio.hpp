#pragma once

#include <SDL3/SDL.h>
#include <vector>

/*
   Specs are defined
*/
struct SoundWAV {
    Uint8* data;
    Uint32 size;
    SDL_AudioStream* audio_stream;

    SoundWAV(std::vector<SDL_AudioStream*>& streams);
    void play_queued();
};
