#pragma once
#include "glm.hpp"
#include "camera.h"

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
    alignas(16) vec4 pos = {0,0,0,0};
    alignas(16) vec4 color = {1,1,1,1};
    float intensity = 1.0f;
    float radius = 10.0f;
};


struct BoundingSphere
{
    glm::vec3 center{ 0.f, 0.f, 0.f };
    float radius{ 0.f };

    BoundingSphere(const glm::vec3& inCenter, float inRadius)
        : center{ inCenter }, radius{ inRadius }
    {}

    bool isOnOrForwardPlan(const Plan& plan) const
    {
        return plan.getSignedDistanceToPlan(center) > -radius;
    }

    bool isOnFrustum(const Frustum& camFrustum) const
    {
        return (isOnOrForwardPlan(camFrustum.leftFace) &&
                isOnOrForwardPlan(camFrustum.rightFace) &&
                isOnOrForwardPlan(camFrustum.farFace) &&
                isOnOrForwardPlan(camFrustum.nearFace) &&
                isOnOrForwardPlan(camFrustum.topFace) &&
                isOnOrForwardPlan(camFrustum.bottomFace));
    };
};
