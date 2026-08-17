#version 460
#pragma shader_stage(vertex)

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_uv;

layout (location = 0) out vec2 v_uv;

layout (binding = 0, set = 1) uniform Uniform_Hud {
    mat4 model;
} uni;

void main() {
    gl_Position = uni.model * vec4(a_position, 0, 1.0);
    v_uv = a_uv;
}
