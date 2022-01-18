#version 430 core

layout(location = 0) out vec4 o_Color;

in vec2 o_TexCoord;

uniform vec3 u_CameraPosition;
uniform vec3 u_DirLightDirection;
uniform float u_DirLightIntensity;
uniform vec3 u_DirLightCol;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform float u_nearPlane;
uniform float u_farPlane;

layout(binding = 0) uniform sampler2DArray shadowMap;
layout(binding = 1) uniform sampler2D depthMap;

uniform int u_cascadeCount;
uniform float u_cascadePlaneDistances[20];

layout (std430, binding = 1) buffer LightSpaceMatrices
{
    mat4 lightSpaceMatrices[];
};

float linearizeDepth(float z)
{
    return (2*u_nearPlane ) / (u_farPlane + u_nearPlane - z*(u_farPlane -u_nearPlane)) ;
}

vec3 reconstructPosition(vec2 uv, float z, mat4 invVP)
{
    vec4 position_s = vec4(uv * 2.0 - vec2(1.0), 2.0 * z - 1.0, 1.0);
    vec4 position_v = invVP * position_s;
    return position_v.xyz / position_v.w;
}

float phase(vec3 inDir, vec3 outDir) 
{
	float cosAngle = dot(inDir, outDir) / (length(inDir) * length(outDir));
	float nom = 3.0 * (1.0 + cosAngle * cosAngle);
	float denom = 16.0 * 3.141592;
	return nom / denom;
}

float dirLightShadow(vec3 fPos);
//temp
int samples = 50;
float g_factor = 0.01;
float fog_strength = 0.3;
float fog_y = 5;

float dither_pattern[16] = float[16] (
	0.0, 0.5, 0.125, 0.625,
	0.75, 0.22, 0.875, 0.375,
	0.1875, 0.6875, 0.0625, 0.5625,
	0.9375, 0.4375, 0.8125, 0.3125
);

float ComputeScattering(float lightDotView)
{
    float result = 1.0f - g_factor * g_factor;
    result /= (4.0f * 3.14159265359 * pow(1.0f + g_factor * g_factor - (2.0f * g_factor) * lightDotView, 1.5f));
    return result;
}

void main()
{
    float depth = texture(depthMap, o_TexCoord, 0).r;
    vec3 worldPos = reconstructPosition(o_TexCoord, depth, inverse(u_Projection * u_View));

    float dither_value = dither_pattern[ (int(gl_FragCoord.x) % 4)* 4 + (int(gl_FragCoord.y) % 4) ];


    vec3 startPosition = u_CameraPosition;

    vec3 rayVector = worldPos - startPosition;

    float rayLength = length(rayVector);
    vec3 rayDirection = rayVector / rayLength;

    float stepLength = rayLength / samples;

    vec3 oneStep = rayDirection * stepLength;
    startPosition+=oneStep+dither_value;

    vec3 currentPosition = startPosition;


    vec3 L = vec3(0.0, 0.0, 0.0);

    float extraFog = 0;

    if(worldPos.y < fog_y)
        extraFog = mix(fog_strength, 0.01, clamp(worldPos.y/fog_y, 0, 1));

    for (int i = 0; i < samples; i++) 
    { 
        float shadow = dirLightShadow(currentPosition);

        if(shadow == 1 )
        {
            float lightDot = dot(normalize(rayDirection), normalize(-u_DirLightDirection));
            L += ComputeScattering(lightDot)*u_DirLightIntensity*u_DirLightCol*(extraFog*u_DirLightCol)*10;
        }

            L += extraFog*u_DirLightIntensity;

        currentPosition += oneStep;
    }

    o_Color = vec4(L/samples, 1);
    // o_Color = vec4(rayCoord, 1);
}

float dirLightShadow(vec3 fPos)
{
    vec4 pos = u_View * vec4(fPos, 1.0);
    float depth = abs(pos.z);

    int layer = -1;
    // layer = 1;
    for (int i = 0; i < u_cascadeCount; ++i)
        if (depth < u_cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }

    if (layer == -1)
        layer = u_cascadeCount;
    
    vec4 posLightSpace = lightSpaceMatrices[layer] * vec4(fPos, 1.0);
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    if (currentDepth  > 1.0)
        return 0.0;

    // PCF
    float shadow = 0.0;

    float sDepth = texture(shadowMap, vec3(projCoords.xy, layer)).r; 
    shadow = currentDepth > sDepth ? 1.0 : 0.0;        

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}