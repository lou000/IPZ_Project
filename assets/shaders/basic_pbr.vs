//type vertex
#version 430 core
layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out vec4 v_Normal;
out vec2 v_TexCoords;
out vec4 v_Pos;
out vec4 v_Color;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform vec4 u_Color;
uniform int u_DrawInstanced;

layout (std430, binding = 2) buffer InstancedTransforms
{
    mat4 instancedTransforms[];
};

void main()
{
    v_Color = u_Color;
    
    mat4 transform = instancedTransforms[gl_InstanceID] *u_Model; //one of these will be set to identity

    v_TexCoords = a_TexCoords;
    v_Pos = vec4(transform * vec4(a_Position.xyz, 1.0));

    v_Normal = transform * a_Normal;   

    gl_Position =  u_Projection * u_View * vec4(v_Pos.xyz, 1.0);
}