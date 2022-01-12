#pragma once

#include "entt.hpp"
#include "math.h"
#include "../AssetManagement/asset_manager.h"
#include "../Renderer/light.h"

struct TransformComponent
{
    vec3 pos =      {0, 0, 0};
    vec3 scale =    {1, 1, 1};
    quat rotation = {1,0,0,0};
    TransformComponent(vec3 pos, vec3 scale, quat rotation)
        :pos(pos), scale(scale), rotation(rotation){}
    TransformComponent(){}
    mat4 transform()
    {
        return translate(mat4(1.0f), pos) * glm::scale(mat4(1.0f),scale)
               * glm::toMat4(rotation);
    }
};

struct RenderSpecComponent
{
    // without this component or light component
    // render pipeline will ignore the entity
    // for now its only color, there will come more
    RenderSpecComponent(){}
    RenderSpecComponent(vec4 col) : color(col){}
    vec4 color = {0,0,0,0};
};

struct MeshComponent
{
    MeshComponent(std::shared_ptr<Model> m) : model(m){}
    MeshComponent(const std::string& meshName)
    {
        model = AssetManager::getAsset<Model>(meshName);
    }
    std::shared_ptr<Model> model;
};

struct PointLightComponent
{
    PointLightComponent(){}
    PointLightComponent(vec3 pos, vec3 color, float intensity, float radius,
                        bool shadowCasting = false)
    {
        light.pos = vec4(pos, 1.f);
        light.color = vec4(color, 1.f);
        light.intensity = intensity;
        light.radius = radius;
        this->shadowCasting = shadowCasting;
    }
    GPU_PointLight light;
    bool shadowCasting = true;
};

struct CameraComponent
{

};

struct CameraControllerComponent
{

};


