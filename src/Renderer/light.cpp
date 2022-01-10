#include "light.h"

GPU_PointLight PointLight::toGPULight()
{
    GPU_PointLight light;
    light.pos = vec4(pos, 1.0f);
    light.color = color;
    light.radius = radius;
    light.intensity = intensity;

    return light;
}
