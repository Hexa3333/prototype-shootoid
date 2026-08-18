#pragma once

#include "zombie.hpp"
#include <vector>

struct Game {
    Game();
    Game(SDL_GPUDevice* _device, SDL_Window* _window, std::shared_ptr<TextureBuffer> _zombie_texture, SDL_GPUSampler* _sampler, std::shared_ptr<Pipeline> _pipeline, SDL_GPUColorTargetDescription* _color_target_desc);

    struct WaveData {
        Uint8 wave;
        Uint32 count;
        Uint16 attack_power;
        float speed;
    };

    std::vector<WaveData> wave_data;
    void read_wave_data();

    // TODO: Zombie should have its own texture already
    std::shared_ptr<TextureBuffer> zombie_texture;

    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_GPUViewport viewport;
    SDL_GPUSampler* sampler;
    std::array<SDL_GPUColorTargetDescription, 2> color_target_desc;
    std::array<SDL_GPUColorTargetInfo, 2> color_target_infos;
    std::shared_ptr<Pipeline> pipeline;

    VertexBuffer hud_vertex_buffer;
    IndexBuffer hud_index_buffer;
    TextureBuffer hud_texture_buffer, hud_texture_buffer2;
    Pipeline* hud_pipeline;
    void upload_hud_buffers();
    void draw_hud(SDL_GPUCommandBuffer* command_buffer, SDL_GPUColorTargetInfo* color_target_info);

    void upload_zombie_buffers();

    std::array<glm::vec3, 4> spawn_points = {
        glm::vec3(-5, 0, 0),
        glm::vec3(0,  5, 0),
        glm::vec3(5, 0, 0),
        glm::vec3(0, -5, 0)
    };

    void send_next_wave();
    void update_zombies();
    void release_zombies();
    void draw_zombies(SDL_GPUCommandBuffer* command_buffer, SDL_GPUColorTargetInfo* color_target_info, SDL_GPUDepthStencilTargetInfo stencil_target_info, glm::mat4 view, glm::mat4 projection);

    int wave_counter;
    std::vector<Zombie*> zombies;
};
