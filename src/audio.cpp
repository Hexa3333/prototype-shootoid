#include "audio.hpp"

SoundWAV::SoundWAV(std::vector<SDL_AudioStream*>& streams) {
    SDL_AudioSpec _nophono;
    if (!SDL_LoadWAV("assets/sounds/snake_eater.wav", &_nophono, &data, &size)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return;
    }

    static SDL_AudioSpec _accepted_audio_spec = {
        .format = SDL_AUDIO_S16LE,
        .channels = 2,
        .freq = 48000
    };

    streams.push_back(SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_accepted_audio_spec, nullptr, nullptr));
    if (!streams.back()) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
    }
    SDL_ResumeAudioStreamDevice(streams.back());
}

void SoundWAV::play_queued() {
    if (SDL_GetAudioStreamQueued(audio_stream) < (int)size) {
        SDL_PutAudioStreamData(audio_stream, data, size);
    }
}
