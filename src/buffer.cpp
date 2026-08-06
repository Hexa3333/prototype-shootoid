#include "buffer.hpp"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

// Vertex Buffer
Buffer::Buffer(SDL_GPUDevice* device, const std::vector<float>& data) {
    size = data.size() * sizeof(float);
    this->device = device;

    create_vertex_buffer();
    create_upload_transfer_buffer();

    stage_for_upload(data);
    auto location = get_location();
    auto region = get_region();
}

void Buffer::upload(SDL_GPUCopyPass* copy_pass) {
    auto location = get_location();
    auto region = get_region();
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
}

void Buffer::bind_vertex_buffer(SDL_GPURenderPass* render_pass) {
    SDL_GPUBufferBinding buffer_binding = {
        .buffer = buffer.get(),
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
}

void Buffer::create_vertex_buffer() {
    SDL_GPUBufferCreateInfo info;
    info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    info.size = size;
    info.props = 0;

    buffer = std::unique_ptr<SDL_GPUBuffer, BufferDeleter>(SDL_CreateGPUBuffer(device, &info), BufferDeleter{device});
    SDL_SetGPUBufferName(device, buffer.get(), "Created in class");
}

void Buffer::create_upload_transfer_buffer() {

    SDL_GPUTransferBufferCreateInfo info;
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = size;
    info.props = 0;
    
    transfer_buffer = std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter>(SDL_CreateGPUTransferBuffer(device, &info), TransferBufferDeleter{device});
}

void Buffer::create_vertex_buffer_descriptions() {
    SDL_GPUVertexBufferDescription description1;
    description1.slot = 0;
    description1.pitch = 3 * sizeof(float);
    description1.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    description1.instance_step_rate = 0;

    vertex_buffer_descriptions.push_back(description1);
}

void Buffer::create_vertex_buffer_attributes() {
    SDL_GPUVertexAttribute position;
    position.location = 0;
    position.buffer_slot = 0;
    position.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    position.offset = 0;

    vertex_attributes.push_back(position);
}

SDL_GPUTransferBufferLocation Buffer::get_location() const {
    SDL_GPUTransferBufferLocation location;
    location.transfer_buffer = transfer_buffer.get();
    location.offset = 0;
    return location;
}
SDL_GPUBufferRegion Buffer::get_region() const {
    SDL_GPUBufferRegion region;
    region.buffer = buffer.get();
    region.size = size;
    region.offset = 0;
    return region;
}

void Buffer::stage_for_upload(const std::vector<float>& data) {
    void* ptr = SDL_MapGPUTransferBuffer(device, transfer_buffer.get(), false);
    SDL_memcpy(ptr, data.data(), size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer.get());
}
