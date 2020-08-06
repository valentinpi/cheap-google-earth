#version 460 core

in vec3 immediate_color;
out vec4 color;

void main()
{
    color = vec4(immediate_color, 1.0);
}
