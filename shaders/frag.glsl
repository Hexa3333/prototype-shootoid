#version 460

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D tex_sampler;

void main()
{
    FragColor = texture(tex_sampler, v_uv);
}
