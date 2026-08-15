#pragma once

#include "zombie.hpp"
#include <vector>
struct Game {
    Game();
    Game(SDL_GPUDevice* _device, SDL_Window* _window, std::shared_ptr<TextureBuffer> _zombie_texture, SDL_GPUSampler* _sampler, std::shared_ptr<Pipeline> _pipeline);
    Game& operator=(Game&& _game);

    // TODO: Zombie should have its own texture already
    std::shared_ptr<TextureBuffer> zombie_texture;

    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_GPUSampler* sampler;
    //std::array<SDL_GPUColorTargetDescription, 2> color_target_desc;
    //std::array<SDL_GPUColorTargetInfo, 2> color_target_infos;
    std::shared_ptr<Pipeline> pipeline;

    void upload_buffers();

    std::array<glm::vec3, 4> spawn_points = {
        glm::vec3(-5, 0, 0),
        glm::vec3(0,  5, 0),
        glm::vec3(5, 0, 0),
        glm::vec3(0, -5, 0)
    };

    void send_next_wave();
    void update_zombies();
    void draw_zombies(SDL_GPUCommandBuffer* command_buffer, SDL_GPUColorTargetInfo* color_target_info, SDL_GPUDepthStencilTargetInfo stencil_target_info, glm::mat4 view, glm::mat4 projection, SDL_GPUViewport* viewport);

    int wave_counter;
    std::vector<Zombie> zombies;
};
