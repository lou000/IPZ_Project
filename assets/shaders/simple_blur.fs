#version 430 core
layout(location = 0) out vec4 FragColor;
  
in vec2 o_TexCoord;
uniform int kernelSize;
int halfKernelSize = kernelSize/2;

layout(binding = 0) uniform sampler2D inputTex;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(inputTex, 0));
    float result = 0.0;
    for (int x = -halfKernelSize; x < halfKernelSize; ++x) 
    {
        for (int y = -halfKernelSize; y < halfKernelSize; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(inputTex, o_TexCoord + offset).r;
        }
    }
    result = result / (kernelSize * kernelSize);
    FragColor = vec4(vec3(result), 1);
}  