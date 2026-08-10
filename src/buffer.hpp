#pragma once

#include <SDL3/SDL_gpu.h>

#include <vector>
#include <memory>

// DRAFT:
/*
class Buffer {
protected:
    virtual void upload(SDL_GPUCopyPass* copy_pass) = 0;

protected:
    SDL_GPUDevice* device;
};
*/


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

struct TextureBufferDeleter {
    SDL_GPUDevice* device;

    void operator()(SDL_GPUTexture* buffer) const {
        SDL_ReleaseGPUTexture(device, buffer);
    };
};

// NOTE: Vertex oriented as of yet
// TODO: Derived classes(?) for other stuff
class VertexBuffer {
public:
    VertexBuffer(SDL_GPUDevice* device, const std::vector<float>& data);
    void upload(SDL_GPUCopyPass* copy_pass);
    void bind(SDL_GPURenderPass* render_pass);
    
    const std::vector<SDL_GPUVertexBufferDescription>& get_vertex_buffer_descriptions() const;
    const std::vector<SDL_GPUVertexAttribute>& get_vertex_attributes() const;

    void draw(SDL_GPURenderPass* render_pass);
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
    SDL_GPUDevice* device;
    Uint32 size;
    Uint32 pitch;
    std::unique_ptr<SDL_GPUBuffer, BufferDeleter> buffer;
    std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter> transfer_buffer;

    std::vector<SDL_GPUVertexBufferDescription> vertex_buffer_descriptions;
    std::vector<SDL_GPUVertexAttribute> vertex_attributes;
};


class IndexBuffer {
public:
    IndexBuffer(SDL_GPUDevice* device, const std::vector<Uint32>& data);
    void upload(SDL_GPUCopyPass* copy_pass);
    void bind(SDL_GPURenderPass* render_pass);
    void draw(SDL_GPURenderPass* render_pass);
private:
    // TODO: Name your buffers
    void create_index_buffer();
    void create_upload_transfer_buffer();
    SDL_GPUTransferBufferLocation get_location() const;
    SDL_GPUBufferRegion get_region() const;

    // Only uploads in a copy pass
    void stage_for_upload(const std::vector<Uint32>& data);

private:
    SDL_GPUDevice* device;
    Uint32 size;
    Uint32 pitch;
    std::unique_ptr<SDL_GPUBuffer, BufferDeleter> buffer;
    std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter> transfer_buffer;
};

class TextureBuffer {
public:
    TextureBuffer(SDL_GPUDevice* device, SDL_Surface* surface);

    void upload(SDL_GPUCopyPass* copy_pass);
    void bind(SDL_GPURenderPass* render_pass, SDL_GPUSampler * sampler);
private:
    void create_texture_buffer();
    void create_upload_transfer_buffer();

    SDL_GPUTextureTransferInfo get_transfer_info() const;
    SDL_GPUTextureRegion get_region() const;

    // Only uploads in a copy pass
    // NOTE: Pixel formats matter
    void stage_for_upload();
private:
    SDL_GPUDevice* device;
    Uint32 size;
    std::unique_ptr<SDL_GPUTexture, TextureBufferDeleter> texture;
    std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter> transfer_buffer;
    SDL_Surface* surface;
};
