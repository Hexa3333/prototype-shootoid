#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <vector>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "buffer.hpp"

constexpr bool _DEBUG = true;

SDL_Window* window;
SDL_GPUDevice* device;

static int indices[] = {
    0, 1, 2,
    2, 3, 0
};

std::vector<float> quad_vertices = {
     0.5f,  0.5f,  0.0f, // top right
    -0.5f,  0.5f,  0.0f, // top left
     0.5f, -0.5f,  0.0f, // bot right

     0.5f, -0.5f,  0.0f, // bot right
    -0.5f,  0.5f,  0.0f, // top left
    -0.5f, -0.5f,  0.0f, // bot left
};

Buffer* buffer_test;
Shader* shader_test;

SDL_GPUBuffer* index_buffer;
SDL_GPUGraphicsPipeline* graphics_pipeline;
SDL_GPUTexture* texture;
SDL_GPUSampler* sampler;
SDL_GPUTexture* depth_texture;

struct Vector {
    float x, y, z;
};
Vector vector;

struct Uniform {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
} uniform;


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
    vector.x = 0; vector.y = 0; vector.z = 0;

    SDL_GPUBufferCreateInfo index_buffer_info{};
    index_buffer_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer_info.size = sizeof(indices);
    index_buffer_info.props = 0;

    index_buffer = SDL_CreateGPUBuffer(device, &index_buffer_info);
    SDL_SetGPUBufferName(device, index_buffer, "Quad IB");

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


    SDL_Surface* surf = SDL_LoadPNG("assets/elf.png");
    if (!surf) {
        std::cerr << "Failed to load image.\n";
        return SDL_APP_FAILURE;
    }
    SDL_Surface* rgba = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA8888);
    SDL_DestroySurface(surf);

    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.type = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texture_info.width = rgba->w;
    texture_info.height = rgba->h;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    texture_info.props = 0;

    texture = SDL_CreateGPUTexture(device, &texture_info);
    SDL_SetGPUTextureName(device, texture, "Albedo");

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
    texture_ti.pixels_per_row = 0;
    texture_ti.rows_per_layer = 0;

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

    buffer_test = new Buffer(device, quad_vertices);

    buffer_test->upload(copy_pass);
    SDL_UploadToGPUBuffer(copy_pass, &index_tblocation, &index_bregion, false);
    SDL_UploadToGPUTexture(copy_pass, &texture_ti, &texture_region, false);

    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(device);

    SDL_ReleaseGPUTransferBuffer(device, index_transfer_buffer);
    SDL_ReleaseGPUTransferBuffer(device, texture_transfer_buffer);
    SDL_DestroySurface(rgba);

    Shader shader(device, "shaders/vertex.spv", "shaders/frag.spv",
            0, 1, 0, 0,
            1, 0, 0, 0
            );

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
                .color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A,
                .enable_blend = true,
            }
            });


    shader_test = new Shader(device, "shaders/test_vertex.spv", "shaders/test_frag.spv",
            0,1,0,0,
            0,0,0,0);

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.vertex_shader = shader_test->vertex_shader;
    pipeline_info.fragment_shader = shader_test->fragment_shader;
    pipeline_info.vertex_input_state = {
        .vertex_buffer_descriptions = buffer_test->get_vertex_buffer_descriptions().data(),
        .num_vertex_buffers = 1,

        .vertex_attributes = buffer_test->get_vertex_attributes().data(),
        .num_vertex_attributes = 1
    };
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.rasterizer_state = {
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_NONE,
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
    };
    pipeline_info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    pipeline_info.depth_stencil_state.enable_depth_test = true;
    pipeline_info.depth_stencil_state.enable_depth_write = true;

    pipeline_info.target_info.color_target_descriptions = &color_target_desc[0];
    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    pipeline_info.target_info.has_depth_stencil_target = true;

    pipeline_info.props = 0;

    graphics_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    if (!graphics_pipeline) {
        std::cerr << "Failed to create graphics pipeline: " << SDL_GetError();
    }

    SDL_GPUTextureCreateInfo depth_texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = 720,
        .height = 480,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
    };

    depth_texture = SDL_CreateGPUTexture(device, &depth_texture_info);

    uniform.model = glm::mat4(1.0f);
    uniform.model = glm::rotate(uniform.model, glm::radians(45.0f), glm::vec3(1.0f,0,0));

    uniform.view = glm::mat4(1.0f);
    uniform.view = glm::translate(uniform.view, glm::vec3(0, 0, -3.0f));

    uniform.projection = glm::mat4(1.0f);
    uniform.projection = glm::perspective(glm::radians(45.0f), 720.0f / 480.0f, 0.1f, 100.0f);

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

    // TODO: Fix Depth Buffer
    SDL_GPUDepthStencilTargetInfo stencil_target_info = {
        .texture = depth_texture,
        .clear_depth = 1.0f,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_DONT_CARE,
        .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
        .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
    };


    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, &stencil_target_info);
    SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);

    SDL_GPUViewport viewport = {
        .x = 0, .y = 0, .w = (float)width, .h = (float)height,
        .min_depth = 0.0f, .max_depth = 1.0f
    };
    SDL_SetGPUViewport(render_pass, &viewport);


    static float rot = 0.0f;
    uniform.model = glm::rotate(glm::mat4(1.0f), glm::radians(rot), glm::vec3(1.0f,0,0));
    rot += 1.0f;
    // Uniform
    SDL_PushGPUVertexUniformData(command_buffer, 0, &uniform, sizeof(Uniform));
    /*
    SDL_PushGPUVertexUniformData(command_buffer, 0, glm::value_ptr(uniform.model), sizeof(glm::mat4));
    SDL_PushGPUVertexUniformData(command_buffer, 0, glm::value_ptr(uniform.view), sizeof(glm::mat4));
    SDL_PushGPUVertexUniformData(command_buffer, 0, glm::value_ptr(uniform.projection), sizeof(glm::mat4));
    */

    // NOTE: Buffers only show up in frames (renderdoc) if they're bound

    buffer_test->bind_vertex_buffer(render_pass);

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
    SDL_BindGPUFragmentSamplers(render_pass, 0, &tex_binding, 1);

    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
    //SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);

    SDL_EndGPURenderPass(render_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUTexture(device, depth_texture);

        std::cout << "Size changed: (" << event->window.data1 << ", " << event->window.data2 << ")\n";

        SDL_GPUTextureCreateInfo depth_texture_info = {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
            .width = (Uint32)event->window.data1,
            .height = (Uint32)event->window.data2,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
        };


        depth_texture = SDL_CreateGPUTexture(device, &depth_texture_info);

        return SDL_APP_CONTINUE;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_W) {
            vector.y += 0.05f;
        } else if (event->key.key == SDLK_S) {
            vector.y -= 0.05f;
        }

        if (event->key.key == SDLK_D) {
            vector.x += 0.05f;
        } else if (event->key.key == SDLK_A) {
            vector.x -= 0.05f;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_WaitForGPUIdle(device);

    SDL_ReleaseGPUTexture(device, depth_texture);
    SDL_ReleaseGPUBuffer(device, index_buffer);
    SDL_ReleaseGPUGraphicsPipeline(device, graphics_pipeline);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}
