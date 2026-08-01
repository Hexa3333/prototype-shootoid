#pragma once

#include <SDL3/SDL_gpu.h>
#include <string>

struct Shader {
    SDL_GPUShader* vertex_shader;
    SDL_GPUShader* fragment_shader;
    // FIX: prefer shared pointers
    SDL_GPUDevice* device;

    Shader(SDL_GPUDevice* device, const std::string& vertex_spv_path, const std::string& fragment_spv_path,
            int num_samplers=0, int num_uniform_buffers=0, int num_storage_buffers=0, int num_storage_textures=0);
    ~Shader();
};
