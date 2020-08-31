#version 460 core

layout (location = 0) in vec3 pos_attr;
layout (location = 1) in vec3 col_attr;

out vec3 immediate_color;

uniform mat4 model = mat4(1.0);
uniform mat4 view = mat4(1.0);
uniform mat4 proj = mat4(1.0);

void main()
{
    gl_Position = proj * view * model * vec4(pos_attr, 1.0);
    immediate_color = col_attr;
}
