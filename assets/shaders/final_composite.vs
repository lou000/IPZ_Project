//type vertex
#version 430 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoord;

out vec2 o_TexCoord;

void main(){
    o_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}