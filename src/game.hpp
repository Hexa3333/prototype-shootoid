#pragma once

#include "buffer.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "camera.hpp"

struct Game {
    std::vector<VertexBuffer> buffers;
    std::vector<IndexBuffer> indexes;
    std::vector<TextureBuffer> textures;
    SDL_GPUSampler* sampler;
    SDL_GPUTexture* depth_texture;
    std::vector<UniformMVP> uniform;
    std::vector<Shader> shaders;
    Camera camera;
    std::vector<Pipeline> pipelines;

    void upload_all_to_gpu();
};
