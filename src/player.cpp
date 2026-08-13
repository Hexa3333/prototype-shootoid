#include "player.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <memory>

static std::vector<float> player_vertices = {
    // Front face (+Z)
    -1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f,

    // Back face (-Z)
     1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f,

    // Left face (-X)
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,

    // Right face (+X)
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 0.0f,

    // Top face (+Y)
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,

    // Bottom face (-Y)
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,

};

static std::vector<Uint32> player_indices = {
    0, 1, 2,   2, 3, 0,      // Front
    4, 5, 6,   6, 7, 4,      // Back
    8, 9, 10,  10, 11, 8,    // Left
    12, 13, 14, 14, 15, 12,  // Right
    16, 17, 18, 18, 19, 16,  // Top
    20, 21, 22, 22, 23, 20,  // Bottom
};


Player::Player(SDL_GPUDevice* device, std::shared_ptr<TextureBuffer> _texture_buffer, SDL_GPUSampler* sampler, std::shared_ptr<Pipeline>pipeline)
    : max_health (100), health (100),
      max_speed  (100), speed  (100),
      max_stamina(100), stamina(100),
      look_direction(glm::vec3(0,0.8,0.4)),
      max_turn_speed(100), turn_speed(100),
      GameObject(std::make_shared<VertexBuffer>(device, player_vertices),
                 std::make_shared<IndexBuffer> (device, player_indices)  ,
                 _texture_buffer,
                 sampler,
                 pipeline) {

}

// Own command_buffer - Only upload vbuffer and ibuffer, since texture is shared
void Player::upload_buffers(SDL_GPUDevice* device) {
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    vertex_buffer->upload(copy_pass);
    index_buffer->upload(copy_pass);

    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(device);
}

void Player::update(glm::mat4 model) {
    uniform_mvp.model = model;

    // TODO view = global camera view
    // TODO view = global projection
    // Do other things
}

void Player::update(glm::vec3 _pos) {
    position = _pos;
    uniform_mvp.model = glm::translate(glm::mat4(1.0f), _pos);
    // TODO view = global camera view
    // TODO view = global projection
    // Do other things
}

void Player::draw(SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass, SDL_GPUViewport* viewport) {
    SDL_BindGPUGraphicsPipeline(render_pass, static_cast<SDL_GPUGraphicsPipeline*>(*pipeline.get()));
    SDL_SetGPUViewport(render_pass, viewport);
    texture_buffer->bind(render_pass, sampler);
    uniform_mvp.push(command_buffer);
    vertex_buffer->bind(render_pass);
    index_buffer->bind(render_pass);

    index_buffer->draw(render_pass);
}
