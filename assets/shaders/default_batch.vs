//type vertex
#version 450

layout(location = 0) in vec4  a_Position;
layout(location = 1) in vec4  a_Color;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TexLayer;
layout(location = 4) in float a_TexIndex;
layout(location = 5) in float a_TilingFactor;

out vec4 v_Color;
out vec2 v_TexCoord;
out flat float v_TexIndex;
out flat float v_TexLayer;
out float v_TilingFactor;

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
    v_TexLayer = a_TexLayer;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	gl_Position = a_Position;
}
