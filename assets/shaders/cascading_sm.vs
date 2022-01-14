#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
    
uniform mat4 u_Model;
uniform int u_DrawInstanced;

layout (std430, binding = 2) buffer InstancedTransforms
{
    mat4 instancedTransforms[];
};

    
void main()
{
    mat4 transform;
    if(u_DrawInstanced>0)
        transform = instancedTransforms[gl_InstanceID];
    else
        transform = u_Model;
        
    gl_Position = transform * vec4(a_Position, 1.0);
}
