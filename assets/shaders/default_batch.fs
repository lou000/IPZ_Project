//type fragment
#version 450


vec4 toGrayscale(in vec4 color)
{
  float average = (color.r + color.g + color.b) / 3.0;
  return vec4(average, average, average, 1.0);
}

vec4 colorize(in vec4 texCol, in vec4 color)
{
    return (toGrayscale(texCol) * color);
}


layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec2 v_TexCoord;
in flat float v_TexIndex;
in flat float v_TexLayer;
in float v_TilingFactor;

layout(location = 0) uniform sampler2D u_Textures[27];
layout(location = 27) uniform sampler2DArray u_TextureArrays[5];

void main()
{
	vec4 texColor;
    vec2 normalUV = v_TexCoord * v_TilingFactor;
    vec3 arrayUV = vec3(v_TexCoord * v_TilingFactor, v_TexLayer);
	switch(int(v_TexIndex))
	{
		case 0:  texColor = texture(u_Textures[0],  normalUV); break;
		case 1:  texColor = texture(u_Textures[1],  normalUV); break;
		case 2:  texColor = texture(u_Textures[2],  normalUV); break;
		case 3:  texColor = texture(u_Textures[3],  normalUV); break;
		case 4:  texColor = texture(u_Textures[4],  normalUV); break;
		case 5:  texColor = texture(u_Textures[5],  normalUV); break;
		case 6:  texColor = texture(u_Textures[6],  normalUV); break;
		case 7:  texColor = texture(u_Textures[7],  normalUV); break;
		case 8:  texColor = texture(u_Textures[8],  normalUV); break;
		case 9:  texColor = texture(u_Textures[9],  normalUV); break;
		case 10: texColor = texture(u_Textures[10], normalUV); break;
		case 11: texColor = texture(u_Textures[11], normalUV); break;
		case 12: texColor = texture(u_Textures[12], normalUV); break;
		case 13: texColor = texture(u_Textures[13], normalUV); break;
		case 14: texColor = texture(u_Textures[14], normalUV); break;
		case 15: texColor = texture(u_Textures[15], normalUV); break;
		case 16: texColor = texture(u_Textures[16], normalUV); break;
		case 17: texColor = texture(u_Textures[17], normalUV); break;
		case 18: texColor = texture(u_Textures[18], normalUV); break;
		case 19: texColor = texture(u_Textures[19], normalUV); break;
		case 20: texColor = texture(u_Textures[20], normalUV); break;
		case 21: texColor = texture(u_Textures[21], normalUV); break;
		case 22: texColor = texture(u_Textures[22], normalUV); break;
		case 23: texColor = texture(u_Textures[23], normalUV); break;
		case 24: texColor = texture(u_Textures[24], normalUV); break;
		case 25: texColor = texture(u_Textures[25], normalUV); break;
		case 26: texColor = texture(u_Textures[26], normalUV); break;
		case 27: texColor = texture(u_TextureArrays[0], arrayUV); break;
		case 28: texColor = texture(u_TextureArrays[1], arrayUV); break;
		case 29: texColor = texture(u_TextureArrays[2], arrayUV); break;
		case 30: texColor = texture(u_TextureArrays[3], arrayUV); break;
		case 31: texColor = texture(u_TextureArrays[4], arrayUV); break;
	}
    texColor*=v_Color;
	color = texColor;
    // if(v_TexIndex == 2)
    //     color = colorize(texColor, vec4(0.965, 0.827, 0.502, 0.95));
    // color = vec4(0.5, 0.2, 0.2, 1);
}
