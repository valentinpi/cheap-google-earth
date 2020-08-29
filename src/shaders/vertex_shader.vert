#version 460 core

layout (location = 0) in vec3 pos_attr;
layout (location = 1) in vec3 col_attr;

out vec3 immediate_color;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(pos_attr, 1.0);
    immediate_color = col_attr;
}
