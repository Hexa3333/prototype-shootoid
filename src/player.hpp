#pragma once

#include <glm/ext/vector_float3.hpp>
#include "gameobject.hpp"

struct Player : public GameObject {
    Player(SDL_GPUDevice* device, std::shared_ptr<TextureBuffer> _texture_buffer, SDL_GPUSampler* sampler, std::shared_ptr<Pipeline> pipeline);
    
    // DON'T FORGET
    void upload_buffers(SDL_GPUDevice* device);
    void draw(SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass, SDL_GPUViewport* viewport) override;

    int max_health;
    int health;

    int max_speed;
    int speed;

    int max_stamina;
    int stamina;

    glm::vec3 look_direction;
    int max_turn_speed;
    int turn_speed;
};
