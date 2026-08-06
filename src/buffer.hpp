#pragma once

#include <SDL3/SDL_gpu.h>

#include <vector>
#include <memory>


// NOTE: Vertex oriented as of yet
// TODO: Derived classes(?) for other stuff
class Buffer {
public:
    Buffer(SDL_GPUDevice* device, const std::vector<float>& data);
    void upload(SDL_GPUCopyPass* copy_pass);
    void bind_vertex_buffer(SDL_GPURenderPass* render_pass);
private:
    // TODO: Name your buffers
    void create_vertex_buffer();
    void create_upload_transfer_buffer();
    void create_vertex_buffer_descriptions();
    void create_vertex_buffer_attributes();
    SDL_GPUTransferBufferLocation get_location() const;
    SDL_GPUBufferRegion get_region() const;

    // Only uploads in a copy pass
    void stage_for_upload(const std::vector<float>& data);

private:
    // TODO: Device to shared_ptr
    SDL_GPUDevice* device;
    Uint32 size;

    struct BufferDeleter {
        SDL_GPUDevice* device;

        void operator()(SDL_GPUBuffer* buffer) const {
            SDL_ReleaseGPUBuffer(device, buffer);
        };
    };

    struct TransferBufferDeleter {
        SDL_GPUDevice* device;

        void operator()(SDL_GPUTransferBuffer* buffer) const {
            SDL_ReleaseGPUTransferBuffer(device, buffer);
        };
    };

private:
    std::unique_ptr<SDL_GPUBuffer, BufferDeleter> buffer;
    std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter> transfer_buffer;

    std::vector<SDL_GPUVertexBufferDescription> vertex_buffer_descriptions;
    std::vector<SDL_GPUVertexAttribute> vertex_attributes;
};
