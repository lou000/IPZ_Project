#version 430 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_BloomColor;

in vec4 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec3 v_Pos;
uniform float u_bloomTreshold;
uniform float u_emissiveIntensity;

vec4 safe_color(vec4 c)
{
  return clamp(c, vec4(0.0), vec4(1e20));
}

void main(){

    vec4 color = safe_color(v_Color);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));

    o_Color = vec4(color.rgb, 1);
    if( brightness >= u_bloomTreshold ){
        o_BloomColor = vec4(color.rgb, 1);
    }
    else
    {
        o_BloomColor =  vec4(vec3(0.0), 1.0);;
    }

}