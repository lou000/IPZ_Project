#pragma once
#include "glm.hpp"
#include "../Core/entity.h"

using namespace glm;


struct DirectionalLight
{
    vec3 direction;
    vec3 color;

    bool enabled = false;
    float intensity = 1.0f;
};

struct GPU_PointLight
{
    vec4 pos = {0,0,0,0};
    vec4 color = {1,1,1,1};
    float intensity = 1.0f;
    float radius = 10.0f;
};

struct PointLight : public Entity
{
    PointLight():Entity(Entity::PointLight){}
    PointLight(vec3 pos, vec4 color, float intensity, float radius );
    bool shadowCasting = true;
    float intensity = 1.0f;
    float radius = 10.0f;
    GPU_PointLight toGPULight();
};

inline PointLight::PointLight(vec3 pos, vec4 color, float intensity, float radius)
    : Entity(Entity::PointLight)
{
    this->pos = pos;
    this->color = color;
    this->intensity = intensity;
    this->radius = radius;
}
