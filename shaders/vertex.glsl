#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;

layout (location = 0) out vec2 v_uv;

layout (binding = 0, set = 1) uniform Uniform {
    mat4 model;
    mat4 view;
    mat4 projection;
} uni;

layout (binding = 1, set = 1) uniform Test {
    float extra;
} test;

void main()
{
    gl_Position = uni.projection * uni.view * uni.model * vec4(a_position + test.extra, 1.0);
    v_uv = a_uv;
}
