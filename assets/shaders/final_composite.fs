//type fragment
#version 430 core
out vec4 o_Color;

in vec2 o_TexCoord;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D ssaoTexture;
layout(binding = 2) uniform sampler2D bloomTexture;
layout(binding = 3) uniform sampler2D volumetricTexture;

uniform float u_bloomIntensity;

vec3 toneMapUncharted2(vec3 color);
vec3 toneMapLottes(vec3 x);
vec3 toneMapFilmic(vec3 x);
vec3 toneMapACES(vec3 x);
vec3 ACESFitted(vec3 color);
vec3 SSAO(vec2 uv);

void main(){
    vec3 volumetric = texture(volumetricTexture, o_TexCoord).rgb;
    vec3 hdrCol = texture(screenTexture, o_TexCoord).rgb;
    vec3 bloom  = texture(bloomTexture, o_TexCoord).rgb;
    float ssao  = texture(ssaoTexture, o_TexCoord).r;
    vec3 color  = hdrCol;
    color      *= ssao;
    color      += volumetric;
    color      += bloom*u_bloomIntensity;

    color = toneMapACES(color);

    o_Color = vec4(color, 1) ;
}


//=================================================================================================
//
//  Baking Lab
//  by MJP and David Neubelt
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================

// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

const mat3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInputMat*color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat*color;

    // Clamp to [0, 1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

vec3 toneMapACES(vec3 x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 toneMapLottes(vec3 x) {
    // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
    const float a = 1.6;
    const float d = 0.977;
    const float hdrMax = 8.0;
    const float midIn = 0.18;
    const float midOut = 0.267;

    // Can be precomputed
    const float b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const float c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(x, vec3(a)) / (pow(x, vec3(a*d)) * b + c);
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