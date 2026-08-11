#include "buffer.hpp"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <glm/ext/vector_float3.hpp>
#include <memory>

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

// May be problematic (WARN)
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

VertexBufferInstanced::VertexBufferInstanced(SDL_GPUDevice* _device, const std::vector<float>& _data, const std::vector<float>& instance_data) 
{
    this->device = _device;
    size = _data.size() * sizeof(float);
    instance_size = instance_data.size() * sizeof(float);

    create_vertex_buffer();
    create_upload_transfer_buffer();

    create_instance_buffer();
    create_instance_upload_transfer_buffer();

    stage_for_upload(_data, instance_data);

    create_vertex_buffer_attributes();
    create_vertex_buffer_descriptions();
}
void VertexBufferInstanced::upload(SDL_GPUCopyPass* copy_pass) {
    auto vlocation = get_location();
    auto vregion = get_region();
    SDL_UploadToGPUBuffer(copy_pass, &vlocation, &vregion, false);


    auto ilocation = get_instance_location();
    auto iregion = get_instance_region();
    SDL_UploadToGPUBuffer(copy_pass, &ilocation, &iregion, false);
}

void VertexBufferInstanced::bind(SDL_GPURenderPass* render_pass) {
    SDL_GPUBufferBinding buffer_binding[] = {
        {
            .buffer = buffer.get(),
            .offset = 0
        },
        {
            .buffer = instance_buffer.get(),
            .offset = 0
        }
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, buffer_binding, 2);
}

void VertexBufferInstanced::draw(SDL_GPURenderPass* render_pass) {
    SDL_DrawGPUPrimitives(render_pass, 6, 3, 0, 0);
}

void VertexBufferInstanced::create_instance_buffer() {
    SDL_GPUBufferCreateInfo info;
    info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    info.size = instance_size;
    info.props = 0;

    instance_buffer = std::unique_ptr<SDL_GPUBuffer, BufferDeleter>(SDL_CreateGPUBuffer(device, &info), BufferDeleter{device});
    SDL_SetGPUBufferName(device, instance_buffer.get(), "IB created in class");
}

void VertexBufferInstanced::create_instance_upload_transfer_buffer() {
    SDL_GPUTransferBufferCreateInfo info;
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = instance_size;
    info.props = 0;
    
    instance_transfer_buffer = std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter>(SDL_CreateGPUTransferBuffer(device, &info), TransferBufferDeleter{device});
}

void VertexBufferInstanced::create_vertex_buffer_descriptions() {
    SDL_GPUVertexBufferDescription description1;
    description1.slot = 0;
    description1.pitch = (pitch = 5 * sizeof(float));
    description1.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    description1.instance_step_rate = 0;

    SDL_GPUVertexBufferDescription description2;
    description2.slot = 1;
    description2.pitch = (instance_pitch = 3 * sizeof(float));
    description2.input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;
    description2.instance_step_rate = 0;

    vertex_buffer_descriptions.push_back(description1);
    vertex_buffer_descriptions.push_back(description2);
}

void VertexBufferInstanced::create_vertex_buffer_attributes() {
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

    SDL_GPUVertexAttribute instance;
    instance.location = 2;
    instance.buffer_slot = 1;
    instance.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    instance.offset = 0;

    vertex_attributes.push_back(position);
    vertex_attributes.push_back(uv);
    vertex_attributes.push_back(instance);
}

SDL_GPUTransferBufferLocation VertexBufferInstanced::get_instance_location() const {
    SDL_GPUTransferBufferLocation location;
    location.transfer_buffer = instance_transfer_buffer.get();
    location.offset = 0;
    return location;
}
SDL_GPUBufferRegion VertexBufferInstanced::get_instance_region() const {
    SDL_GPUBufferRegion region;
    region.buffer = instance_buffer.get();
    region.size = instance_size;
    region.offset = 0;
    return region;
}

void VertexBufferInstanced::stage_for_upload(const std::vector<float>& data, const std::vector<float>& instance_data) {
    void* ptr = SDL_MapGPUTransferBuffer(device, transfer_buffer.get(), false);
    SDL_memcpy(ptr, data.data(), size);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer.get());

    void* ptr2 = SDL_MapGPUTransferBuffer(device, instance_transfer_buffer.get(), false);
    SDL_memcpy(ptr2, instance_data.data(), instance_size);
    SDL_UnmapGPUTransferBuffer(device, instance_transfer_buffer.get());
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

// May be problematic (WARN)
void IndexBuffer::draw(SDL_GPURenderPass* render_pass) {
    Uint32 num_indices = size / sizeof(Uint32);
    SDL_DrawGPUIndexedPrimitives(render_pass, num_indices, 2, 0, 0, 0);
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


TextureBuffer::TextureBuffer(SDL_GPUDevice* _device, SDL_Surface* _surface)
 : device(_device), surface(_surface) {
     create_texture_buffer();
     create_upload_transfer_buffer();

     stage_for_upload();
}

void TextureBuffer::upload(SDL_GPUCopyPass* copy_pass) {
    auto source = get_transfer_info();
    auto destination = get_region();
    SDL_UploadToGPUTexture(copy_pass, &source, &destination, false);
}

void TextureBuffer::bind(SDL_GPURenderPass* render_pass, SDL_GPUSampler* sampler) {
    SDL_GPUTextureSamplerBinding binding;
    binding.texture = texture.get();
    binding.sampler = sampler;
    SDL_BindGPUFragmentSamplers(render_pass, 0, &binding, 1);
}

void TextureBuffer::create_texture_buffer() {
    SDL_GPUTextureCreateInfo info;
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    // INFO (TODO): Look here:
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    info.width = surface->w;
    info.height = surface->h;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.props = 0;

    texture = std::unique_ptr<SDL_GPUTexture, TextureBufferDeleter>(SDL_CreateGPUTexture(device, &info));
    SDL_SetGPUTextureName(device, texture.get(), "Created in class");
}

void TextureBuffer::create_upload_transfer_buffer() {
    SDL_GPUTransferBufferCreateInfo info;
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = surface->pitch * surface->h;
    info.props = 0;

    transfer_buffer = std::unique_ptr<SDL_GPUTransferBuffer, TransferBufferDeleter>(SDL_CreateGPUTransferBuffer(device, &info));
}


SDL_GPUTextureTransferInfo TextureBuffer::get_transfer_info() const {
    SDL_GPUTextureTransferInfo info;
    info.transfer_buffer = transfer_buffer.get();
    info.offset = 0;
    info.pixels_per_row = 0;
    info.rows_per_layer = 0;
    return info;
}

SDL_GPUTextureRegion TextureBuffer::get_region() const {
    SDL_GPUTextureRegion region;
    region.texture = texture.get();
    region.mip_level = 0;
    region.layer = 0;
    region.x = 0;
    region.y = 0;
    region.z = 0;
    region.w = surface->w;
    region.h = surface->h;
    region.d = 1;
    return region;
}

void TextureBuffer::stage_for_upload() {
    void* ptr = SDL_MapGPUTransferBuffer(device, transfer_buffer.get(), false);
    SDL_memcpy(ptr, surface->pixels, surface->pitch * surface->h);
    SDL_UnmapGPUTransferBuffer(device, transfer_buffer.get());
}
