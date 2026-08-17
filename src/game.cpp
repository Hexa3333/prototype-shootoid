#include "game.hpp"
#include <SDL3/SDL_gpu.h>
#include <array>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <glm/ext/matrix_transform.hpp>

Game::Game() {

    // HUD
    /*
    Shader hud_shader(device, "build/shaders/hud_vertex.spv", "build/shaders/hud_frag.spv",
            0,1,0,0,
            1,0,0,0);
    auto hud_pipeline1 = std::make_unique<Pipeline>(device, hud_vertex_buffer.get(), hud_shader, &color_target_desc[1]);
    std::unique_ptr<VertexBuffer> hud_vertex_buffer;
    std::unique_ptr<IndexBuffer> hud_index_buffer;
    std::unique_ptr<TextureBuffer> hud_texture_buffer;
    */
}

Game::Game(SDL_GPUDevice* _device, SDL_Window* _window, std::shared_ptr<TextureBuffer> _zombie_texture, SDL_GPUSampler* _sampler, std::shared_ptr<Pipeline> _pipeline, std::unique_ptr<Pipeline> _hud_pipeline)
 : wave_counter(0) {
     device = _device;
     window = _window;
     zombie_texture = _zombie_texture;
     sampler = _sampler;
     pipeline = _pipeline;
     hud_pipeline = std::move(_hud_pipeline);
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
    hud_pipeline = std::move(_game.hud_pipeline);
    wave_counter = _game.wave_counter;
    zombies = _game.zombies;
    wave_data = _game.wave_data;

    return *this;
}

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

void Game::draw_hud(SDL_GPUCommandBuffer* command_buffer, SDL_GPUColorTargetInfo* color_target_info) {
    SDL_GPURenderPass* hud_render_pass = SDL_BeginGPURenderPass(command_buffer, color_target_info, 1, nullptr);
    SDL_BindGPUGraphicsPipeline(hud_render_pass, static_cast<SDL_GPUGraphicsPipeline*>(*hud_pipeline));
    SDL_SetGPUViewport(hud_render_pass, &viewport);

    hud_vertex_buffer->bind(hud_render_pass);
    hud_index_buffer->bind(hud_render_pass);
    hud_texture_buffer->bind(hud_render_pass, sampler);

    glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-0.6f, 0.8, 0));
    trans = glm::scale(trans, glm::vec3(0.3f, 0.1f, 1.0f));
    UniformHUD hud = {
        .model = trans
    };
    hud.push(command_buffer);

    hud_index_buffer->draw(hud_render_pass);

    static float damaging = 0.0f;
    hud_texture_buffer->bind(hud_render_pass, sampler);
    trans = glm::translate(glm::mat4(1.0f), glm::vec3(-0.52f + damaging, 0.8, 0));
    trans = glm::scale(trans, glm::vec3(0.2f - damaging, 0.06f, 1.0));
    UniformHUD hud_2 = {
        .model = trans
    };
    hud_2.push(command_buffer);
    hud_index_buffer->draw(hud_render_pass);
    damaging += 0.0005f;

    SDL_EndGPURenderPass(hud_render_pass);
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
