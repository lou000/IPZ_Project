//type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec4 a_Color;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform vec4 u_Color;

out vec4 v_Color;
out vec3 v_Normal;
out vec3 v_Pos;


void main(){
    v_Normal = mat3(u_Model) * a_Normal;
    mat4 viewModel = u_View * u_Model;
    v_Pos =  vec3(viewModel*vec4(a_Position, 1));
    gl_Position =  u_Projection * (viewModel*vec4(a_Position, 1));
    v_Color = u_Color*a_Color;
}
