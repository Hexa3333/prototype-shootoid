#include "zombie.hpp"
#include "gameobject.hpp"


static std::vector<float> zombie_vertices = {
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

static std::vector<Uint32> zombie_indices = {
    0, 1, 2,   2, 3, 0,      // Front
    4, 5, 6,   6, 7, 4,      // Back
    8, 9, 10,  10, 11, 8,    // Left
    12, 13, 14, 14, 15, 12,  // Right
    16, 17, 18, 18, 19, 16,  // Top
    20, 21, 22, 22, 23, 20,  // Bottom
};


Zombie::Zombie(SDL_GPUDevice* device, std::shared_ptr<TextureBuffer> _texture_buffer, SDL_GPUSampler* sampler, std::shared_ptr<Pipeline> pipeline)
     : GameObject(std::make_shared<VertexBuffer>(device, zombie_vertices),
                 std::make_shared<IndexBuffer> (device, zombie_indices)  ,
                 _texture_buffer,
                 sampler,
                 pipeline) {
}

void Zombie::upload_buffers(SDL_GPUDevice* device) {
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    vertex_buffer->upload(copy_pass);
    index_buffer->upload(copy_pass);

    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_WaitForGPUIdle(device);
}
