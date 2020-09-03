#version 460 core

in vec3 pos_attr;
in vec2 tex_attr;

out vec2 immediate_texcoord;

uniform mat4 model = mat4(1.0);
uniform mat4 view = mat4(1.0);
uniform mat4 proj = mat4(1.0);

void main()
{
    gl_Position = proj * view * model * vec4(pos_attr, 1.0);
    immediate_texcoord = tex_attr;
}
