#version 460
#pragma shader_stage(fragment)

layout (location = 0) in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

layout (binding = 0, set = 2) uniform sampler2D tex_sampler;

void main() {
    FragColor = texture(tex_sampler, v_uv);
}
