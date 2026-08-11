#include "pipeline.hpp"
#include <SDL3/SDL_gpu.h>

#include <iostream>

Pipeline::Pipeline(SDL_GPUDevice* _device, VertexBuffer* buffer, Shader* shader, SDL_GPUColorTargetDescription* desc, SDL_GPUFillMode fill_mode) {
    device = _device;

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = shader->vertex_shader;
    info.fragment_shader = shader->fragment_shader;
    info.vertex_input_state = {
        .vertex_buffer_descriptions = buffer->get_vertex_buffer_descriptions().data(),
        .num_vertex_buffers = (Uint32)buffer->get_vertex_buffer_descriptions().size(),
        .vertex_attributes = buffer->get_vertex_attributes().data(),
        .num_vertex_attributes = (Uint32)buffer->get_vertex_attributes().size()
    };
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state = {
        .fill_mode = fill_mode,
        .cull_mode = SDL_GPU_CULLMODE_NONE,
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
    };
    info.depth_stencil_state = {
        .compare_op = SDL_GPU_COMPAREOP_LESS,
        .enable_depth_test = true,
        .enable_depth_write = true,
    };
    info.target_info = {
        .color_target_descriptions = desc,
        .num_color_targets = 1,
        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .has_depth_stencil_target = true
    };

    info.props = 0;

    pipeline_obj = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline_obj) {
        std::cerr << "Failed to create graphics pipeline: " << SDL_GetError();
    }
}

Pipeline::~Pipeline() {
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline_obj);
}
