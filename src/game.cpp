#include "game.hpp"
#include <SDL3/SDL_gpu.h>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

Game::Game() {}

Game::Game(SDL_GPUDevice* _device, SDL_Window* _window, std::shared_ptr<TextureBuffer> _zombie_texture, SDL_GPUSampler* _sampler, std::shared_ptr<Pipeline> _pipeline)
 : wave_counter(0) {
     device = _device;
     window = _window;
     zombie_texture = _zombie_texture;
     sampler = _sampler;
     pipeline = _pipeline;
     zombies.clear();

     read_wave_data();
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
    wave_data = _game.wave_data;

    return *this;
}

#include <iostream>
void Game::read_wave_data() {
    std::ifstream input("assets/waves.csv");
    if (!input.is_open()) {
        std::cout << "Failed to read waves.csv\n";
    }

    std::vector<std::vector<std::string>> lines;
    for (std::string line; std::getline(input, line);) {
        std::istringstream ss(std::move(line));
        std::vector<std::string> row;

        for (std::string value; std::getline(ss, value, ',');) {
            row.push_back(std::move(value));
        }

        lines.push_back(std::move(row));
    }

    for (auto& line : lines) {
        // Skip if #
        if (line[0][0] == '#') {
            continue;
        }

        WaveData _wave_data;
        _wave_data.wave = static_cast<Uint8>(std::stoi(line[0]));
        _wave_data.count = static_cast<Uint32>(std::stoi(line[1]));
        _wave_data.attack_power = static_cast<Uint16>(std::stoi(line[2]));
        _wave_data.speed = std::stof(line[3]);

        wave_data.push_back(_wave_data);
    }
}

// Too expensive
void Game::upload_buffers() {
    for (auto& z : zombies) {
        z->upload_buffers(device);
    }
}

void Game::release_zombies() {
    for (auto& z : zombies) {
        delete z;
    }
    zombies.clear();
}

void Game::send_next_wave() {
    if (wave_counter >= 4) {
        return;
    }
    SDL_WaitForGPUIdle(device);
 
    WaveData& wd = wave_data.at(wave_counter);
    std::cout << "Sending wave" << wd.wave << ":\n"
        "\tcount: " << wd.count << "\n"
        "\tattack_power: " << wd.attack_power << "\n"
        "\tspeed: " << wd.speed << "\n";

    release_zombies();
    for (int i = 0; i < wd.count; ++i) {
        Zombie* new_zombie = new Zombie(device, zombie_texture, sampler, pipeline);
        new_zombie->upload_buffers(device);
        zombies.push_back(new_zombie);
    }

    ++wave_counter;
}

// TODO
void Game::update_zombies() {
}

void Game::draw_zombies(SDL_GPUCommandBuffer* command_buffer, SDL_GPUColorTargetInfo* color_target_info, SDL_GPUDepthStencilTargetInfo stencil_target_info, glm::mat4 view, glm::mat4 projection) {
    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, color_target_info, 1, &stencil_target_info);

    float offset_x = 0.0f;
    for (auto& z : zombies) {
        z->update(spawn_points.at(wave_counter-1) + glm::vec3(-offset_x,0,0));
        offset_x += 2.0f;
        z->uniform_mvp.view = view;
        z->uniform_mvp.projection = projection;
        z->draw(command_buffer, render_pass, &viewport);
    }
    SDL_EndGPURenderPass(render_pass);
}
