#version 460

layout (location = 0) in vec3 a_position;

layout (binding = 0, set = 1) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main() {
    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(a_position, 1.0);
}
