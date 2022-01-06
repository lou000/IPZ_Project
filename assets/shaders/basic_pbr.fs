//type fragment
#version 430 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_BloomColor;

in vec4 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_Pos;

// material parameters
uniform float u_Metallic;
uniform float u_Roughness;

// lights
struct PointLight{
    vec4 position;
    vec4 color;
    bool enabled;
    bool shadowCasting;
    float intensity;
    float range;
};

layout (std430, binding = 2) buffer pointLightsSSBO
{
    PointLight pointLights[];
};

uniform vec3 u_CameraPosition;
uniform vec3 u_DirLightDirection;
uniform float u_DirLightIntensity;
uniform float u_bloomTreshold;
uniform float u_exposure;
uniform vec3 u_DirLightCol;
uniform uint u_PointLightCount;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float NdotV, float NdotL, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 dirLightContribution(vec3 lightDirection, vec3 lightColor, float intensity,
                          vec3 viewDir, vec3 normal, vec3 color, float rough, float metal, vec3 F0);
vec3 pointLightContribution(vec3 lightPosition, vec3 lightColor, float intensity, float range,
                            vec3 fragPos, vec3 viewDir, vec3 normal, vec3 color, float rough, float metal, vec3 F0);

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 V = normalize(u_CameraPosition - v_Pos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, v_Color.rgb, u_Metallic);

    // lights contribution TODO: pass intensities and ranges
    vec3 Lo = dirLightContribution(u_DirLightDirection, u_DirLightCol, u_DirLightIntensity, V, N, v_Color.rgb, u_Roughness, u_Metallic, F0);

    for(int i = 0; i < u_PointLightCount; ++i)
    {
        vec3 pos = pointLights[i].position.xyz;
        vec3 col = pointLights[i].color.rgb;
        Lo += pointLightContribution(pos, col, pointLights[i].intensity, pointLights[i].range,
                                     v_Pos, V, N, v_Color.rgb, u_Roughness, u_Metallic, F0);
    }

    vec3 ambient = vec3(0.3) * v_Color.rgb;
    vec3 color = ambient + Lo;

    color*=u_exposure;
    o_Color = vec4(color, v_Color.a);
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    if( brightness >= u_bloomTreshold ){
        o_BloomColor = vec4(color, 1.0);
    }
    else
    {
        o_BloomColor = vec4(vec3(0.0), 1.0);
    }
}

vec3 pointLightContribution(vec3 lightPosition, vec3 lightColor, float intensity, float range,
                            vec3 fragPos, vec3 viewDir, vec3 normal, vec3 color, float rough, float metal, vec3 F0)
{
    vec3 lightDir = normalize(lightPosition - fragPos);
    vec3 halfway = normalize(viewDir + lightDir);
    float distance    = length(lightPosition - fragPos);
    float attenuation = pow(clamp(1 - pow((distance / range), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance) );
    vec3 radiance     = lightColor * attenuation;

    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    float NDF = DistributionGGX(normal, halfway, rough);
    float G   = GeometrySmith(nDotV, nDotL, rough);
    vec3  F   = FresnelSchlick(max(dot(halfway,viewDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max(denominator, 0.0000001);

    radiance *= (kD * (color / PI) + specular ) * nDotL;

    return intensity*radiance;
}

vec3 dirLightContribution(vec3 lightDirection, vec3 lightColor, float intensity,
                          vec3 viewDir, vec3 normal, vec3 color, float rough, float metal, vec3 F0)
{
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(viewDir + L);
    vec3 radiance = lightColor;
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, L), 0.0);

    // cook-torrance brdf
    float NDF = DistributionGGX(normal, H, rough);
    float G   = GeometrySmith(nDotV, nDotL, rough);
    vec3  F   = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);

    //Finding specular and diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL + 0.0001;
    vec3  specular    = numerator / max(denominator, 0.0001);

    radiance *= (kD * (color / PI) + specular) *nDotL;

    return intensity*radiance;
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
