#pragma once

#include "buffer.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <SDL3/SDL_gpu.h>
#include <memory>

class GameObject {
public:
    GameObject(std::shared_ptr<VertexBuffer> vbuffer,
               std::shared_ptr<IndexBuffer> ibuffer,
               std::shared_ptr<TextureBuffer> tbuffer,
               SDL_GPUSampler* _sampler,
               std::shared_ptr<Pipeline> _pipeline);

    virtual void update(glm::mat4 model);
    virtual void update(glm::vec3 _pos);
    glm::vec3 get_position() const;
    virtual void draw(SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass, SDL_GPUViewport* viewport);
    UniformMVP uniform_mvp;
protected:
    std::shared_ptr<VertexBuffer> vertex_buffer;
    std::shared_ptr<IndexBuffer> index_buffer;
    // TODO: TextureBuffer could get coupled with samplers...
    std::shared_ptr<TextureBuffer> texture_buffer;
    SDL_GPUSampler* sampler; // DANGER
    std::shared_ptr<Pipeline> pipeline;
    
    glm::vec3 position;
};
