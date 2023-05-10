#version 450 core

layout (location = 0) in vec2 pos;
layout (location=1) in vec3 in_color;

layout (location=0) out vec3 out_color;
uniform mat4 model;
void main(void) 
{
    gl_Position =model* vec4(pos,0,1);
    out_color = in_color;
}

