//type vertex
#version 430 core
layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

out vec3 v_Normal;
out vec2 v_TexCoords;
out vec3 v_Pos;
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

uniform int u_offsetToHeightMap;
layout(binding = 5) uniform sampler2D noiseTexture;
ivec2 meshSize = ivec2(200, 200);
vec2 meshPos = vec2(-100, -100);
float amplitude = 30;

void main()
{
    v_Color = u_Color;
    
    mat4 transform = instancedTransforms[gl_InstanceID] *u_Model; //one of these will be set to identity

    v_TexCoords = a_TexCoords;
    v_Pos = vec3(transform * vec4(a_Position.xyz, 1.0));

    v_Normal = mat3(transform) * a_Normal.xyz;   

    gl_Position =  u_Projection * u_View * vec4(v_Pos, 1.0);
}