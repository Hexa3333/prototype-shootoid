#include "buffer.hpp"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

// Vertex Buffer
VertexBuffer::VertexBuffer(SDL_GPUDevice* device, const std::vector<float>& data) {
    this->device = device;
    size = data.size() * sizeof(float);

    create_vertex_buffer();
    create_upload_transfer_buffer();

    stage_for_upload(data);

    create_vertex_buffer_attributes();
    create_vertex_buffer_descriptions();
}

void VertexBuffer::upload(SDL_GPUCopyPass* copy_pass) {
    auto location = get_location();
    auto region = get_region();
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
}

void VertexBuffer::bind(SDL_GPURenderPass* render_pass) {
    SDL_GPUBufferBinding buffer_binding = {
        .buffer = buffer.get(),
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
}

const std::vector<SDL_GPUVertexBufferDescription>& VertexBuffer::get_vertex_buffer_descriptions() const {
    return vertex_buffer_descriptions;
}

const std::vector<SDL_GPUVertexAttribute>& VertexBuffer::get_vertex_attributes() const {
    return vertex_attributes;
}

// May be problematic (doubt)
void VertexBuffer::draw(SDL_GPURenderPass* render_pass) {
    //Uint32 num_vertices = size / pitch;
    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
}

void VertexBuffer::create_vertex_buffer() {
    SDL_GPUBufferCreateInfo info;
    info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    info.size = size;
    info.props = 0;

    buffer = std::unique_ptr<SDL_GPUBuffer, BufferDeleter>(SDL_CreateGPUBuffer(device, &info), BufferDeleter{device});
    SDL_SetGPUBufferName(device, buffer.get(), "Created in class");
}

void VertexBuffer::create_upload_transfer_buffer() {

    SDL_GPUTransferBufferCreateInfo info;
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = size;
    info.props = 0;
    
    transfer_buffer = std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter>(SDL_CreateGPUTransferBuffer(device, &info), TransferBufferDeleter{device});
}

void VertexBuffer::create_vertex_buffer_descriptions() {
    SDL_GPUVertexBufferDescription description1;
    description1.slot = 0;
    description1.pitch = (pitch = 5 * sizeof(float));
    description1.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    description1.instance_step_rate = 0;

    vertex_buffer_descriptions.push_back(description1);
}

void VertexBuffer::create_vertex_buffer_attributes() {
    SDL_GPUVertexAttribute position;
    position.location = 0;
    position.buffer_slot = 0;
    position.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    position.offset = 0;

    SDL_GPUVertexAttribute uv;
    uv.location = 1;
    uv.buffer_slot = 0;
    uv.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    uv.offset = 3 * sizeof(float);

    vertex_attributes.push_back(position);
    vertex_attributes.push_back(uv);
}

SDL_GPUTransferBufferLocation VertexBuffer::get_location() const {
    SDL_GPUTransferBufferLocation location;
    location.transfer_buffer = transfer_buffer.get();
    location.offset = 0;
    return location;
}
SDL_GPUBufferRegion VertexBuffer::get_region() const {
    SDL_GPUBufferRegion region;
    region.buffer = buffer.get();
    region.size = size;
    region.offset = 0;
    return region;
}

void VertexBuffer::stage_for_upload(const std::vector<float>& data) {
    void* ptr = SDL_MapGPUTransferBuffer(device, transfer_buffer.get(), false);
    SDL_memcpy(ptr, data.data(), size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer.get());
}



IndexBuffer::IndexBuffer(SDL_GPUDevice* device, const std::vector<Uint32>& data) {
    this->device = device;
    size = data.size() * sizeof(Uint32);

    create_index_buffer();
    create_upload_transfer_buffer();

    stage_for_upload(data);
}

void IndexBuffer::upload(SDL_GPUCopyPass* copy_pass) {
    auto location = get_location();
    auto region = get_region();
    SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
}

void IndexBuffer::bind(SDL_GPURenderPass* render_pass) {
    SDL_GPUBufferBinding buffer_binding = {
        .buffer = buffer.get(),
        .offset = 0
    };
    SDL_BindGPUIndexBuffer(render_pass, &buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
}

void IndexBuffer::create_index_buffer() {
    SDL_GPUBufferCreateInfo info;
    info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    info.size = size;
    info.props = 0;

    buffer = std::unique_ptr<SDL_GPUBuffer, BufferDeleter>(SDL_CreateGPUBuffer(device, &info), BufferDeleter{device});
    SDL_SetGPUBufferName(device, buffer.get(), "Created in class - Index Buffer");
}

void IndexBuffer::create_upload_transfer_buffer() {
    SDL_GPUTransferBufferCreateInfo info;
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.props = 0;

    transfer_buffer = std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter>(SDL_CreateGPUTransferBuffer(device, &info), TransferBufferDeleter{device});
}

SDL_GPUTransferBufferLocation IndexBuffer::get_location() const {
    SDL_GPUTransferBufferLocation location;
    location.transfer_buffer = transfer_buffer.get();
    location.offset = 0;
    return location;
}

SDL_GPUBufferRegion IndexBuffer::get_region() const {
    SDL_GPUBufferRegion region;
    region.buffer = buffer.get();
    region.size = size;
    region.offset = 0;
    return region;
}

void IndexBuffer::stage_for_upload(const std::vector<Uint32>& data) {
    void* ptr = SDL_MapGPUTransferBuffer(device, transfer_buffer.get(), false);
    SDL_memcpy(ptr, data.data(), size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer.get());
}
