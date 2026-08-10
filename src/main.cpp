#include "pipeline.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
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
#include "camera.hpp"

constexpr bool _DEBUG = true;

SDL_Window* window;
SDL_GPUDevice* device;


std::vector<float> quad_vertices = {
     0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // top right
     0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // bot right
    -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  // bot left
    -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  // top left
};

std::vector<Uint32> quad_indices = {
    0, 1, 3,
    1, 2, 3
};

std::vector<float> cube_vertices = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

std::vector<float> cube_vertices_indexed = {
    // Front face (+Z)
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,

    // Back face (-Z)
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f,

    // Left face (-X)
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,

    // Right face (+X)
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f,

    // Top face (+Y)
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,

    // Bottom face (-Y)
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

};

std::vector<Uint32> cube_indices = {
    0, 1, 2,   2, 3, 0,      // Front
    4, 5, 6,   6, 7, 4,      // Back
    8, 9, 10,  10, 11, 8,    // Left
    12, 13, 14, 14, 15, 12,  // Right
    16, 17, 18, 18, 19, 16,  // Top
    20, 21, 22, 22, 23, 20,  // Bottom
};

VertexBuffer* buffer_test;
IndexBuffer* index_test;
TextureBuffer* texture_test;
Shader* shader_test;
Camera* camera;
Pipeline* pipeline_test, *polygon_pipeline_test;

SDL_GPUTexture* depth_texture;
SDL_GPUSampler* sampler;

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

    SDL_Surface* surf = SDL_LoadPNG("assets/elf.png");
    if (!surf) {
        std::cerr << "Failed to load image.\n";
        return SDL_APP_FAILURE;
    }
    SDL_Surface* rgba = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surf);

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

    buffer_test = new VertexBuffer(device, cube_vertices_indexed);
    index_test = new IndexBuffer(device, cube_indices);
    texture_test = new TextureBuffer(device, rgba);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    buffer_test->upload(copy_pass);
    index_test->upload(copy_pass);
    texture_test->upload(copy_pass);

    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(device);

    SDL_DestroySurface(rgba);

    shader_test = new Shader(device, "shaders/vertex.spv", "shaders/frag.spv",
            0,1,0,0,
            1,0,0,0);

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

    pipeline_test = new Pipeline(device, buffer_test, shader_test, &color_target_desc[0]);
    polygon_pipeline_test = new Pipeline(device, buffer_test, shader_test, &color_target_desc[0], SDL_GPU_FILLMODE_LINE);

    SDL_GPUTextureCreateInfo depth_texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
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

    camera = new Camera();

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

    SDL_GPUColorTargetInfo color_target_infos[2] = {
        {
            .texture = swapchain_texture,
            .clear_color = {255/255.0f, 140/255.0f, 140/255.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        },
        {
            .texture = swapchain_texture,
            .clear_color = {255/255.0f, 190/255.0f, 140/255.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_LOAD,
            .store_op = SDL_GPU_STOREOP_STORE,
        }
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

    // DRAFT: Initial render pass might set things such as the background up,
    // and the following passes do other things.

    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_infos[0], 1, &stencil_target_info);
    SDL_BindGPUGraphicsPipeline(render_pass, static_cast<SDL_GPUGraphicsPipeline*>(*pipeline_test));

    SDL_GPUViewport viewport = {
        .x = 0, .y = 0, .w = (float)width, .h = (float)height,
        .min_depth = 0.0f, .max_depth = 1.0f
    };
    SDL_SetGPUViewport(render_pass, &viewport);

    texture_test->bind(render_pass, sampler);

    static float rot = 0.0f;
    uniform.model = glm::rotate(glm::mat4(1.0f), glm::radians(rot), glm::vec3(1.0f,0,0));
    // Uniform
    uniform.view = camera->update();
    SDL_PushGPUVertexUniformData(command_buffer, 0, &uniform, sizeof(Uniform));

    // NOTE: Buffers only show up in frames (renderdoc) if they're bound

    buffer_test->bind(render_pass);
    index_test->bind(render_pass);

    index_test->draw(render_pass);

    uniform.model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.5f, 0));
    uniform.model = glm::rotate(uniform.model, glm::radians(-rot), glm::vec3(1.0f,0,0));
    SDL_PushGPUVertexUniformData(command_buffer, 0, &uniform, sizeof(Uniform));
    index_test->draw(render_pass);

    SDL_EndGPURenderPass(render_pass);

    // Draw polygonized version to an image?
    SDL_GPURenderPass* polygon_render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_infos[1], 1, &stencil_target_info);
    SDL_BindGPUGraphicsPipeline(polygon_render_pass, static_cast<SDL_GPUGraphicsPipeline*>(*polygon_pipeline_test));
    SDL_SetGPUViewport(polygon_render_pass, &viewport);
    texture_test->bind(render_pass, sampler);

    uniform.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0,0));
    uniform.model = glm::rotate(uniform.model, glm::radians(rot), glm::vec3(1.0f,0,0));

    uniform.view = camera->update();
    SDL_PushGPUVertexUniformData(command_buffer, 0, &uniform, sizeof(Uniform));

    texture_test->bind(render_pass, sampler);

    buffer_test->bind(polygon_render_pass);
    index_test->bind(polygon_render_pass);

    index_test->draw(polygon_render_pass);

    uniform.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.5f, 0));
    uniform.model = glm::rotate(uniform.model, glm::radians(-rot), glm::vec3(1.0f,0,0));
    SDL_PushGPUVertexUniformData(command_buffer, 0, &uniform, sizeof(Uniform));
    index_test->draw(polygon_render_pass);

    SDL_EndGPURenderPass(polygon_render_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);

    rot += 1.0f;
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
            .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
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
            glm::vec3 new_pos = camera->get_position() + glm::vec3(0, 0, 1.0f);
            camera->set_position(new_pos);
        } else if (event->key.key == SDLK_S) {
            glm::vec3 new_pos = camera->get_position() - glm::vec3(0, 0, 1.0f);
            camera->set_position(new_pos);
        }

        if (event->key.key == SDLK_D) {
            glm::vec3 new_pos = camera->get_position() - glm::vec3(1.0f, 0, 0);
            camera->set_position(new_pos);
        } else if (event->key.key == SDLK_A) {
            glm::vec3 new_pos = camera->get_position() + glm::vec3(1.0f, 0, 0);
            camera->set_position(new_pos);
        }


        if (event->key.key == SDLK_UP) {
            glm::vec3 new_pos = camera->get_position() + glm::vec3(0, 1.0f, 0);
            camera->set_position(new_pos);
        } else if (event->key.key == SDLK_DOWN) {
            glm::vec3 new_pos = camera->get_position() - glm::vec3(0, 1.0f, 0);
            camera->set_position(new_pos);
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_WaitForGPUIdle(device);

    SDL_ReleaseGPUTexture(device, depth_texture);
    delete pipeline_test;
    delete polygon_pipeline_test;
    delete texture_test;
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}
