#pragma once
#include "glm.hpp"

using namespace glm;


struct DirectionalLight
{
    vec3 direction;
    vec3 color;

    bool enabled = false;
    float intensity = 1.0f;
    float ambientIntensity = 0.05f;
};

struct GPU_PointLight
{
    vec4 pos = {0,0,0,0};
    vec4 color = {1,1,1,1};
    float intensity = 1.0f;
    float radius = 10.0f;
};


