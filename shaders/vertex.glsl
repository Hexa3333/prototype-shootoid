#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;

layout (location = 0) out vec2 v_uv;

layout (binding = 0, set = 1) uniform Uniform {
    mat4 model;
    mat4 view;
    mat4 projection;
} uni;


void main()
{
    gl_Position = uni.projection * uni.view * uni.model * vec4(a_position, 1.0);
    v_uv = a_uv;
}
