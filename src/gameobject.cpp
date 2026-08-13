#include "gameobject.hpp"
#include <SDL3/SDL_gpu.h>

// WARN (TODO): Is the shared_ptr logic alright?
GameObject::GameObject(std::shared_ptr<VertexBuffer> vbuffer,
               std::shared_ptr<IndexBuffer> ibuffer,
               std::shared_ptr<TextureBuffer> tbuffer,
               SDL_GPUSampler* _sampler,
               std::shared_ptr<Pipeline> _pipeline)
    : vertex_buffer(vbuffer),
      index_buffer(ibuffer),
      texture_buffer(tbuffer),
      sampler(_sampler),
      pipeline(_pipeline) {
}

void GameObject::update(glm::mat4 model) {
    uniform_mvp.model = model;
    // TODO view = global camera view
    // TODO view = global projection
}

void GameObject::draw(SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass, SDL_GPUViewport* viewport) {
    SDL_BindGPUGraphicsPipeline(render_pass, static_cast<SDL_GPUGraphicsPipeline*>(*pipeline.get()));
    SDL_SetGPUViewport(render_pass, viewport);
    texture_buffer->bind(render_pass, sampler);
    uniform_mvp.push(command_buffer);
    vertex_buffer->bind(render_pass);
    index_buffer->bind(render_pass);

    index_buffer->draw(render_pass);
}
