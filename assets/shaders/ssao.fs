//type fragment
#version 430 core

layout(location = 0) out vec4 o_Color;

in vec2 o_TexCoord;
uniform mat4 u_Projection;

layout(binding = 0) uniform sampler2D depthTexture;
layout(binding = 1) uniform sampler2D noiseTexture;

layout (std430, binding = 4) buffer LightSpaceMatrices
{
    vec3 u_Samples[];
};

int kernelSize = 64;
float radius = 0.8;
float bias = 0.03;


vec3 reconstructPosition(vec2 uv, float z, mat4 invVP)
{
    vec4 position_s = vec4(uv * 2.0 - vec2(1.0), 2.0 * z - 1.0, 1.0);
    vec4 position_v = invVP * position_s;
    return position_v.xyz / position_v.w;
}

void main()
{
    vec2 uv = o_TexCoord;
    mat4 invVP = inverse(u_Projection);
    vec2 resolution = textureSize(depthTexture, 0);
    vec2 noiseScale = vec2(resolution.x/4.0, resolution.y/4.0); 
    
    float depth = texture(depthTexture, uv).r;
    vec3 pos = reconstructPosition(uv, depth, invVP);
    vec3 normal = normalize(cross(dFdx(pos), dFdy(pos)));

    vec3 randomVec = normalize(texture(noiseTexture, uv * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; i++)
    {
        vec3 samplePos = TBN * u_Samples[i]; // from tangent to view-space
        samplePos = pos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = u_Projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(depthTexture, offset.xy).r; // get depth value of kernel sample
        vec3 pos2 = reconstructPosition(offset.xy, sampleDepth, invVP);
        sampleDepth = pos2.z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;   
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    o_Color = vec4(vec3(occlusion), 1);
}