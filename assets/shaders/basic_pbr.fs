//type fragment
#version 430 core

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_Pos;

// material parameters
uniform float u_Metallic;
uniform float u_Roughness;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 u_CameraPosition;
uniform vec3 u_DirLightDirection;
uniform vec3 u_DirLightCol;

const float PI = 3.14159265359;

vec3 toneMapUncharted2(vec3 color);
vec3 toneMapFilmic(vec3 x);
vec3 toneMapACES(vec3 x);
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
    vec3 Lo = dirLightContribution(u_DirLightDirection, u_DirLightCol, 5, V, N, v_Color.rgb, u_Roughness, u_Metallic, F0);
    
    for(int i = 0; i < 4; ++i) 
    {
        Lo += pointLightContribution(lightPositions[i], lightColors[i], 1, 20, v_Pos, V, N, v_Color.rgb, u_Roughness, u_Metallic, F0);
    }
  
    vec3 ambient = vec3(0.03) * v_Color.rgb;
    vec3 color = ambient + Lo;
	
    // sdr mapping
    //TODO: Move this to postprocess step
    color = toneMapACES(color);
   
    o_Color = vec4(color, v_Color.a);
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

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 toneMapACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// https://github.com/dmnsgn/glsl-tone-map/blob/master/uncharted2.glsl
vec3 uncharted2Tonemap(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 toneMapUncharted2(vec3 color) {
    const float W = 11.2;
    float exposureBias = 2.0;
    vec3 curr = uncharted2Tonemap(exposureBias * color);
    vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
    return curr * whiteScale;
}

// Filmic Tonemapping Operators http://filmicworlds.com/blog/filmic-tonemapping-operators
vec3 toneMapFilmic(vec3 x) {
    vec3 X = max(vec3(0.0), x - 0.004);
    vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return pow(result, vec3(2.2));
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
