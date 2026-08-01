#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <cstring>
#include <iostream>
#include <vector>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include "shader.hpp"

constexpr bool _DEBUG = true;

SDL_Window* window;
SDL_GPUDevice* device;


static float vertices[] = {
//   x     y     z                                  
    0.5f,  0.5f, 0.0f,       0.0f, 1.0f, 1.0f, 1.0f, // top right
   -0.5f,  0.5f, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f, // top left
   -0.5f, -0.5f, 0.0f,       0.0f, 1.0f, 0.0f, 1.0f, // bot left

    0.5f,  0.5f, 0.0f,       0.0f, 1.0f, 1.0f, 1.0f, // top right
   -0.5f, -0.5f, 0.0f,       0.0f, 1.0f, 0.0f, 1.0f, // bot left
    0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f, 1.0f, // bot right
};

static float vertices_indexed[] = {
//   x     y     z                                  
    0.5f,  0.5f, 0.0f,       0.0f, 1.0f, 1.0f, 1.0f, // top right
   -0.5f,  0.5f, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f, // top left
   -0.5f, -0.5f, 0.0f,       0.0f, 1.0f, 0.0f, 1.0f, // bot left
    0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f, 1.0f, // bot right
};

SDL_GPUBuffer* vertex_buffer;
SDL_GPUTransferBuffer* transfer_buffer;
SDL_GPUGraphicsPipeline* graphics_pipeline;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    std::cout << "Initializing...\n";

    window = SDL_CreateWindow("shootoid", 720, 480, SDL_WINDOW_RESIZABLE);
    if ((device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, _DEBUG, "vulkan")) == NULL) {
        std::cerr << "Failed to create GPU Device.\n";
        return SDL_APP_FAILURE;
    }
    SDL_ClaimWindowForGPUDevice(device, window);

    // Check support
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    bool has_spirv = (formats & SDL_GPU_SHADERFORMAT_SPIRV) != 0;

    bool texture_format_supported = SDL_GPUTextureSupportsFormat(device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
    
    std::cout << "Has spirv: " << has_spirv << "\n"
              << "2D RGB unorm support: " << texture_format_supported << "\n";

    // Vertex buffer
    SDL_GPUBufferCreateInfo buffer_info{};
    buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    buffer_info.size = sizeof(vertices);
    buffer_info.props = 0;

    vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
    SDL_SetGPUBufferName(device, vertex_buffer, "Triangle VB");

    // Transfer buffer
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = sizeof(vertices);
    transfer_info.props = 0;

    transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);

    // Upload data
    {
        float* data = (float*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
        SDL_memcpy(data, vertices, sizeof(vertices));
        SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
    }

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = transfer_buffer;
    location.offset = 0;

    SDL_GPUBufferRegion region{};
    region.buffer = vertex_buffer;
    region.size = sizeof(vertices);
    region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(device);

    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);

    // TODO: Index buffer

    Shader shader(device, "shaders/vertex.spv", "shaders/frag.spv");


    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
    vertex_buffer_descriptions[0].slot = 0;
    vertex_buffer_descriptions[0].pitch = 7 * sizeof(float);
    vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_descriptions[0].instance_step_rate = 0;

    SDL_GPUVertexAttribute vertex_attributes[3];
    // a_position
    vertex_attributes[0].location = 0;
    vertex_attributes[0].buffer_slot = 0;
    vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attributes[0].offset = 0;

    // a_color
    vertex_attributes[1].location = 1;
    vertex_attributes[1].buffer_slot = 0;
    vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    vertex_attributes[1].offset = 3 * sizeof(float);


    std::vector<SDL_GPUColorTargetDescription> color_target_desc;
    color_target_desc.push_back({
            .format = SDL_GetGPUSwapchainTextureFormat(device, window),
            .blend_state = {
                .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .color_blend_op = SDL_GPU_BLENDOP_ADD,
                .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
                .alpha_blend_op = SDL_GPU_BLENDOP_ADD,

                .enable_blend = true,
            }
            });


    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.vertex_shader = shader.vertex_shader;
    pipeline_info.fragment_shader = shader.fragment_shader;
    pipeline_info.vertex_input_state = {
        .vertex_buffer_descriptions = vertex_buffer_descriptions,
        .num_vertex_buffers = 1,

        .vertex_attributes = vertex_attributes,
        .num_vertex_attributes = 2
    };
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.rasterizer_state = {
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_BACK,
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
    };
    pipeline_info.depth_stencil_state = {
        .compare_op = SDL_GPU_COMPAREOP_LESS,
        .enable_depth_test = true,
        .enable_depth_write = true,
    };
    pipeline_info.target_info.color_target_descriptions = &color_target_desc[0];
    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    pipeline_info.target_info.has_depth_stencil_target = true;

    pipeline_info.props = 0;

    graphics_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    if (!graphics_pipeline) {
        std::cerr << "Failed to create graphics pipeline: " << SDL_GetError();
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);

    unsigned int width, height;
    SDL_GPUTexture* swapchain_texture;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, &width, &height);
    if (swapchain_texture == NULL) {
        SDL_SubmitGPUCommandBuffer(command_buffer);
        return SDL_APP_CONTINUE;
    }

    SDL_GPUColorTargetInfo color_target_info = {
        .texture = swapchain_texture,
        .clear_color = {255/255.0f, 140/255.0f, 140/255.0f, 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
    };

    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, NULL);
    SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);

    SDL_GPUViewport viewport = {
        .x = 0, .y = 0, .w = (float)width, .h = (float)height,
        .min_depth = 0.0f, .max_depth = 1.0f
    };
    SDL_SetGPUViewport(render_pass, &viewport);

    SDL_GPUBufferBinding buffer_binding = {
        .buffer = vertex_buffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);

    // Skipped: Texture binding

    // SDL_PushGPUVertexUniformData(command_buffer, Uint32 slot_index, const void *data, Uint32 length)

    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);

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
    SDL_WaitForGPUIdle(device);

    SDL_ReleaseGPUBuffer(device, vertex_buffer);
    SDL_ReleaseGPUGraphicsPipeline(device, graphics_pipeline);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}
