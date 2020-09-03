#version 460 core

in vec2 immediate_texcoord;

out vec4 color;

uniform sampler2D sampler_2d;

void main()
{
    color = texture(sampler_2d, immediate_texcoord);
}
