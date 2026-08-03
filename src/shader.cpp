#include "shader.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <exception>
#include <iostream>


Shader::Shader(SDL_GPUDevice* device, const std::string& vertex_spv_path, const std::string& fragment_spv_path,
               int vertex_num_samplers, int vertex_num_uniform_buffers, int vertex_num_storage_buffers, int vertex_num_storage_textures,
               int frag_num_samplers, int frag_num_uniform_buffers, int frag_num_storage_buffers, int frag_num_storage_textures)
              : device(device) {
    // Vertex
    size_t vertex_code_size;
    unsigned char* vertex_code = (unsigned char*)SDL_LoadFile(vertex_spv_path.data(), &vertex_code_size);
    if (vertex_code == NULL) {
        std::cerr << SDL_GetError();
        std::terminate();
    }

    SDL_GPUShaderCreateInfo vertex_info{};
    vertex_info.code_size = vertex_code_size;
    vertex_info.code = vertex_code;
    vertex_info.entrypoint = "main";
    vertex_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vertex_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vertex_info.num_samplers = vertex_num_samplers;
    vertex_info.num_uniform_buffers = vertex_num_uniform_buffers;
    vertex_info.num_storage_buffers = vertex_num_storage_buffers;
    vertex_info.num_storage_textures = vertex_num_storage_textures;
    vertex_info.props = 0;

    this->vertex_shader = SDL_CreateGPUShader(device, &vertex_info);
    if (vertex_shader == NULL) {
        std::cerr << SDL_GetError();
        std::terminate();
    }

    // Frag
    size_t frag_code_size;
    unsigned char* frag_code = (unsigned char*)SDL_LoadFile(fragment_spv_path.data(), &frag_code_size);
    if (frag_code == NULL) {
        std::cerr << SDL_GetError();
        std::terminate();
    }

    SDL_GPUShaderCreateInfo frag_info{};
    frag_info.code_size = frag_code_size;
    frag_info.code = frag_code;
    frag_info.entrypoint = "main";
    frag_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    frag_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    frag_info.num_samplers = frag_num_samplers;
    frag_info.num_storage_textures = frag_num_storage_textures;
    frag_info.num_storage_buffers = frag_num_storage_buffers;
    frag_info.num_uniform_buffers = frag_num_uniform_buffers;
    frag_info.props = 0;

    this->fragment_shader = SDL_CreateGPUShader(device, &frag_info);
    if (fragment_shader == NULL) {
        std::cerr << SDL_GetError();
        std::terminate();
    }

    SDL_free(vertex_code);
    SDL_free(frag_code);
}

Shader::~Shader() {
    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, fragment_shader);
}
