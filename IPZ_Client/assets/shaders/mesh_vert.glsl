
//type vertex
#version 450

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec3  a_Normal;

uniform mat4 u_ViewProjection;
uniform vec4 u_Color;
out vec4 v_Color;
out vec3 v_Normal;
out vec3 v_Pos;


void main(){
    v_Normal = a_Normal;
    v_Pos = a_Position;
    gl_Position =  u_ViewProjection * vec4(a_Position, 1);
    v_Color = u_Color;
}
