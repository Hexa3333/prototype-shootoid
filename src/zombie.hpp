#pragma once

#include "gameobject.hpp"

class Zombie : public GameObject {
public:
    // TODO: Zombie should have its own texture already
    Zombie(SDL_GPUDevice* device, std::shared_ptr<TextureBuffer> _texture_buffer, SDL_GPUSampler* sampler, std::shared_ptr<Pipeline> pipeline);

    // DON'T FORGET
    void upload_buffers(SDL_GPUDevice* device);

    Uint16 health;
    Uint16 speed;
};
