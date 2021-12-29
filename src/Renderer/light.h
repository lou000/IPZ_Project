#pragma once
#include "glm.hpp"

using namespace glm;




struct DirectionalLight
{
    vec3 direction;
    vec3 color;

    bool enabled = false;
    bool shadowCasting = true;
    float intensity = 1.0f;
};

struct PointLight
{
    vec4 pos = {0,0,0,0};
    vec4 color = {1,1,1,1};

    uint enabled = false;
    uint shadowCasting = true;

    float intensity = 1.0f;
    float range = 10.0f;
};
