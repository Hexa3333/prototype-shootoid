#include "game.hpp"
#include <SDL3/SDL_gpu.h>
#include <array>

Game::Game() {}

Game::Game(SDL_GPUDevice* _device, SDL_Window* _window, std::shared_ptr<TextureBuffer> _zombie_texture, SDL_GPUSampler* _sampler, std::shared_ptr<Pipeline> _pipeline)
 : wave_counter(0) {
     device = _device;
     window = _window;
     zombie_texture = _zombie_texture;
     sampler = _sampler;
     pipeline = _pipeline;
     zombies.clear();
}

Game& Game::operator=(Game&& _game) {
    // TODO: improve
    device = _game.device;
    window = _game.window;
    zombie_texture = _game.zombie_texture;
    sampler = _game.sampler;
    pipeline = _game.pipeline;
    wave_counter = _game.wave_counter;
    zombies = _game.zombies;

    return *this;
}

void Game::upload_buffers() {
    for (auto& z : zombies) {
        z.upload_buffers(device);
    }
}

#include <iostream>
void Game::send_next_wave() {
    SDL_WaitForGPUIdle(device);
    zombies.push_back(Zombie(device, zombie_texture, sampler, pipeline));
    upload_buffers();
    
    std::cout << "Wave " << wave_counter << " sent.\n";
    ++wave_counter;
}

void Game::update_zombies() {
}

void Game::draw_zombies(SDL_GPUCommandBuffer* command_buffer, SDL_GPUColorTargetInfo* color_target_info, SDL_GPUDepthStencilTargetInfo stencil_target_info, glm::mat4 view, glm::mat4 projection) {
    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, color_target_info, 1, &stencil_target_info);
    SDL_GPUViewport viewport = {
        .x = 0, .y = 0, .w = 720.0f, .h = 480.0f,
        .min_depth = 0.0f, .max_depth = 1.0f
    };

    int i = 0;
    for (auto& z : zombies) {
        z.update(spawn_points[i++]);
        z.uniform_mvp.view = view;
        z.uniform_mvp.projection = projection;
        z.draw(command_buffer, render_pass, &viewport);
    }
    SDL_EndGPURenderPass(render_pass);
}
