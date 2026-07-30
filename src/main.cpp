#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <cstring>
#include <iostream>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

constexpr bool _DEBUG = true;

SDL_Window* window;
SDL_GPUDevice* device;

static float vertices[] = {
//   x     y     z     
    0.0f,  0.5f, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f,
   -0.5f, -0.5f, 0.0f,       0.0f, 1.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f, 1.0f,
};

SDL_GPUBuffer* vertex_buffer;
SDL_GPUTransferBuffer* transfer_buffer;
SDL_GPUShader* vertex_shader, *fragment_shader;
SDL_GPUGraphicsPipeline* graphics_pipeline;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    std::cout << "Initializing...\n";

    window = SDL_CreateWindow("shootoid", 720, 480, SDL_WINDOW_RESIZABLE);
    if ((device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, _DEBUG, "vulkan")) == NULL) {
        std::cerr << "Failed to create GPU Device.\n";
        return SDL_APP_FAILURE;
    }
    SDL_ClaimWindowForGPUDevice(device, window);

    SDL_GPUBufferCreateInfo buffer_info{};
    buffer_info.size = sizeof(vertices);
    buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = sizeof(vertices);
    transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);

    float* data = (float*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    SDL_memcpy(data, vertices, sizeof(vertices));

    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    // Shader
    size_t vertex_code_size;
    void* vertex_code = SDL_LoadFile("shaders/vertex.spv", &vertex_code_size);

    SDL_GPUShaderCreateInfo vertex_info{};
    vertex_info.code = (unsigned char*)vertex_code;
    vertex_info.code_size = vertex_code_size;
    vertex_info.entrypoint = "main";
    vertex_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vertex_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vertex_info.num_samplers = 0;
    vertex_info.num_storage_buffers = 0;
    vertex_info.num_storage_textures = 0;
    vertex_info.num_uniform_buffers = 0;
    vertex_shader = SDL_CreateGPUShader(device, &vertex_info);

    size_t fragment_code_size;
    void* fragment_code = SDL_LoadFile("shaders/frag.spv", &fragment_code_size);

    SDL_GPUShaderCreateInfo fragment_info{};
    fragment_info.code = (unsigned char*)fragment_code;
    fragment_info.code_size = fragment_code_size;
    fragment_info.entrypoint = "main";
    fragment_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    fragment_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    fragment_info.num_samplers = 0;
    fragment_info.num_storage_buffers = 0;
    fragment_info.num_storage_textures = 0;
    fragment_info.num_uniform_buffers = 0;
    fragment_shader = SDL_CreateGPUShader(device, &fragment_info);

    SDL_free(vertex_code);
    SDL_free(fragment_code);

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.vertex_shader = vertex_shader;
    pipeline_info.fragment_shader = fragment_shader;
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    
    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
    vertex_buffer_descriptions[0].slot = 0;
    vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_descriptions[0].instance_step_rate = 0;
    vertex_buffer_descriptions[0].pitch = 7 * sizeof(float);

    pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    pipeline_info.vertex_input_state.vertex_buffer_descriptions = vertex_buffer_descriptions;

    SDL_GPUVertexAttribute vertex_attributes[2];
    // a_position
    vertex_attributes[0].buffer_slot = 0;
    vertex_attributes[0].location = 0;
    vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attributes[0].offset = 0;

    // a_color
    vertex_attributes[1].buffer_slot = 0;
    vertex_attributes[1].location = 1;
    vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    vertex_attributes[1].offset = 3 * sizeof(float);

    pipeline_info.vertex_input_state.num_vertex_attributes = 2;
    pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes;

    SDL_GPUColorTargetDescription color_target_desc[1];
    color_target_desc[0] = {};
    color_target_desc[0].format = SDL_GetGPUSwapchainTextureFormat(device, window);

    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = color_target_desc;

    graphics_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, fragment_shader);


    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = transfer_buffer;
    location.offset = 0;

    SDL_GPUBufferRegion region{};
    region.buffer = vertex_buffer;
    region.size = sizeof(vertices);
    region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);

    SDL_GPUColorTargetInfo color_target_info{};
    color_target_info.clear_color = {240/255.0f, 240/255.0f, 240/255.0f, 1.0f};
    color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;

    color_target_info.store_op = SDL_GPU_STOREOP_STORE;

    unsigned int width, height;
    SDL_GPUTexture* swapchain_texture;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, &width, &height);
    if (swapchain_texture == NULL) {
        SDL_SubmitGPUCommandBuffer(command_buffer);
        return SDL_APP_CONTINUE;
    }
    color_target_info.texture = swapchain_texture;

    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, NULL);
    SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);

    SDL_GPUBufferBinding buffer_bindings[1];
    buffer_bindings[0].buffer = vertex_buffer;
    buffer_bindings[0].offset = 0;

    SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings, 1);
    SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_ReleaseGPUBuffer(device, vertex_buffer);
    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
    SDL_ReleaseGPUGraphicsPipeline(device, graphics_pipeline);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}
