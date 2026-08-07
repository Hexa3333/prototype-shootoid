#pragma once

#include "shader.hpp"
#include "buffer.hpp"

class Pipeline {
public:
    // VertexBuffer: to get descriptions and attributes
    Pipeline(SDL_GPUDevice* _device, VertexBuffer* buffer, Shader* shader, SDL_GPUColorTargetDescription* desc);
    operator SDL_GPUGraphicsPipeline*() const { return pipeline_obj; }

private:
    // TODO: Release
    SDL_GPUGraphicsPipeline* pipeline_obj;

    SDL_GPUDevice* device;
};
