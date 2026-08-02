#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
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
//   x     y     z            r     g     b     a         u     v
    0.5f,  0.5f, 0.0f,       0.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,     // top right
   -0.5f,  0.5f, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,     // top left
   -0.5f, -0.5f, 0.0f,       0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 0.0f,     // bot left
    0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 0.0f,     // bot right
};

static int indices[] = {
    0, 1, 2,
    2, 3, 0
};

SDL_GPUBuffer* vertex_buffer, *index_buffer;
SDL_GPUGraphicsPipeline* graphics_pipeline;
SDL_GPUTexture* texture;
SDL_GPUSampler* sampler;

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
    buffer_info.size = sizeof(vertices_indexed);
    buffer_info.props = 0;

    vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
    SDL_SetGPUBufferName(device, vertex_buffer, "Quad VB");

    SDL_GPUBufferCreateInfo index_buffer_info{};
    index_buffer_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer_info.size = sizeof(indices);
    index_buffer_info.props = 0;

    index_buffer = SDL_CreateGPUBuffer(device, &index_buffer_info);
    SDL_SetGPUBufferName(device, index_buffer, "Quad IB");

    // Transfer buffer - vertices
    SDL_GPUTransferBufferCreateInfo vertex_transfer_info{};
    vertex_transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    vertex_transfer_info.size = sizeof(vertices_indexed);
    vertex_transfer_info.props = 0;

    SDL_GPUTransferBuffer* vertex_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &vertex_transfer_info);
    {
        float* data = (float*)SDL_MapGPUTransferBuffer(device, vertex_transfer_buffer, false);
        SDL_memcpy(data, vertices_indexed, sizeof(vertices_indexed));
        SDL_UnmapGPUTransferBuffer(device, vertex_transfer_buffer);
    }

    // Transfer buffer - indices
    SDL_GPUTransferBufferCreateInfo index_transfer_info{};
    index_transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    index_transfer_info.size = sizeof(indices);
    index_transfer_info.props = 0;

    SDL_GPUTransferBuffer* index_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &index_transfer_info);
    {
        float* data = (float*)SDL_MapGPUTransferBuffer(device, index_transfer_buffer, false);
        SDL_memcpy(data, indices, sizeof(indices));
        SDL_UnmapGPUTransferBuffer(device, index_transfer_buffer);
    }

    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.type = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texture_info.width = 512;
    texture_info.height = 512;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    texture_info.props = 0;

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &texture_info);
    SDL_SetGPUTextureName(device, texture, "Albedo");

    SDL_Surface* surf = SDL_LoadPNG("assets/elf.png");
    if (!surf) {
        std::cerr << "Failed to load image.\n";
        return SDL_APP_FAILURE;
    }
    SDL_Surface* rgba = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA8888);
    SDL_DestroySurface(surf);

    SDL_GPUTransferBufferCreateInfo texture_transfer_buffer_info{};
    texture_transfer_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    texture_transfer_buffer_info.size = rgba->pitch * rgba->h;

    SDL_GPUTransferBuffer* texture_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &texture_transfer_buffer_info);
    void* ptr = SDL_MapGPUTransferBuffer(device, texture_transfer_buffer, false);
    SDL_memcpy(ptr, rgba->pixels, rgba->pitch * rgba->h);
    SDL_UnmapGPUTransferBuffer(device, texture_transfer_buffer);

    SDL_GPUSamplerCreateInfo sampler_info{};
    sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    sampler_info.enable_anisotropy = true;
    sampler_info.max_anisotropy = 16.0f;
    sampler_info.mip_lod_bias = 0.0f;
    sampler_info.min_lod = 0.0f;
    sampler_info.max_lod = 1.0f;
    sampler_info.props = 0;

    sampler = SDL_CreateGPUSampler(device, &sampler_info);


    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    // VB transfer
    SDL_GPUTransferBufferLocation vertex_tblocation{};
    vertex_tblocation.transfer_buffer = vertex_transfer_buffer;
    vertex_tblocation.offset = 0;

    SDL_GPUBufferRegion vertex_bregion{};
    vertex_bregion.buffer = vertex_buffer;
    vertex_bregion.size = sizeof(vertices);
    vertex_bregion.offset = 0;

    // IB transfer
    SDL_GPUTransferBufferLocation index_tblocation{};
    index_tblocation.transfer_buffer = index_transfer_buffer;
    index_tblocation.offset = 0;

    SDL_GPUBufferRegion index_bregion{};
    index_bregion.buffer = index_buffer;
    index_bregion.size = sizeof(indices);
    index_bregion.offset = 0;

    // Texture transfer - (PROBLEM HERE?)
    SDL_GPUTextureTransferInfo texture_ti{};
    texture_ti.transfer_buffer = texture_transfer_buffer;
    texture_ti.offset = 0;
    texture_ti.pixels_per_row = rgba->w;
    texture_ti.rows_per_layer = rgba->h;

    SDL_GPUTextureRegion texture_region{};
    texture_region.texture = texture;
    texture_region.mip_level = 0;
    texture_region.layer = 0;
    texture_region.x = 0;
    texture_region.y = 0;
    texture_region.z = 0;
    texture_region.w = rgba->w;
    texture_region.h = rgba->h;
    texture_region.d = 1;

    SDL_UploadToGPUBuffer(copy_pass, &vertex_tblocation, &vertex_bregion, false);
    SDL_UploadToGPUBuffer(copy_pass, &index_tblocation, &index_bregion, false);
    //SDL_UploadToGPUTexture(copy_pass, &texture_ti, &texture_region, false);

    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(device);

    SDL_ReleaseGPUTransferBuffer(device, vertex_transfer_buffer);
    SDL_ReleaseGPUTransferBuffer(device, index_transfer_buffer);
    //SDL_ReleaseGPUTransferBuffer(device, texture_transfer_buffer);
    SDL_DestroySurface(rgba);

    Shader shader(device, "shaders/vertex.spv", "shaders/frag.spv", 0, 1);


    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
    vertex_buffer_descriptions[0].slot = 0;
    vertex_buffer_descriptions[0].pitch = 9 * sizeof(float);
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

    // a_uv
    vertex_attributes[2].location = 2;
    vertex_attributes[2].buffer_slot = 0;
    vertex_attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    vertex_attributes[2].offset = 7 * sizeof(float);


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
        .num_vertex_attributes = 3
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


    // Uniform
    static float x = 0.0f;
    SDL_PushGPUVertexUniformData(command_buffer, 0, &x, sizeof(float));
    x += 0.01f;

    SDL_GPUBufferBinding buffer_binding = {
        .buffer = vertex_buffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);

    SDL_GPUBufferBinding index_buffer_binding = {
        .buffer = index_buffer,
        .offset = 0
    };
    SDL_BindGPUIndexBuffer(render_pass, &index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    // Skipped: Texture binding
    SDL_GPUTextureSamplerBinding tex_binding = {
        .texture = texture,
        .sampler = sampler
    };
    //SDL_BindGPUFragmentSamplers(render_pass, 0, &tex_binding, 1);

    //SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
    SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);

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
    SDL_ReleaseGPUBuffer(device, index_buffer);
    SDL_ReleaseGPUGraphicsPipeline(device, graphics_pipeline);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}
