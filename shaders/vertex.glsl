#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

layout (location = 0) out vec4 v_color;

layout (binding = 0, set = 1) uniform Uniform {
    float plus_x;
} uni;


void main()
{
    gl_Position = vec4(a_position.x + uni.plus_x, a_position.yz, 1.0f);
    v_color = a_color;
}
