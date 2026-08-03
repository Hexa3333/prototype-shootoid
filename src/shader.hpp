#pragma once

#include <SDL3/SDL_gpu.h>
#include <string>

struct Shader {
    SDL_GPUShader* vertex_shader;
    SDL_GPUShader* fragment_shader;
    // FIX: prefer shared pointers
    SDL_GPUDevice* device;

    /* TODO */
    struct Info {
        SDL_GPUShaderStage stage;
        Uint32 num_samplers;
        Uint32 num_uniform_buffers;
        Uint32 num_storage_buffers;
        Uint32 num_storage_textures;
    };

    Shader(SDL_GPUDevice* device, const std::string& vertex_spv_path, const std::string& fragment_spv_path,
            int vertex_num_samplers=0, int vertex_num_uniform_buffers=0, int vertex_num_storage_buffers=0, int vertex_num_storage_textures=0,
            int frag_num_samplers=0, int frag_num_uniform_buffers=0, int frag_num_storage_buffers=0, int frag_num_storage_textures=0);
    ~Shader();
};
