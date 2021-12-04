#pragma once
#include "glm.hpp"

using namespace glm;




struct DirectionalLight
{
    vec3 pos;
    vec3 color;

    bool enabled = true;
    bool shadowCasting = true;
    float intensity = 1.0f;
};

struct PointLight
{
    vec3 pos;
    vec3 col;

    bool enabled = true;
    bool shadowCasting = true;
    float intensity = 1.0f;
    float range = 10.0f;
};
