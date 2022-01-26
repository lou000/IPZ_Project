#pragma once

#include "entt.hpp"
#include "math.h"
#include "../AssetManagement/asset_manager.h"
#include "../Renderer/light.h"
#include "../Audio/AudioSource.h"

struct IDComponent
{
    IDComponent(uint64 id) : m_id(id){}
    uint64 m_id = 0;
    operator uint64() const {return m_id;}
};

struct TagComponent
{
    TagComponent(){}
    TagComponent(const std::string& tag) : tag(tag){}
    std::string tag;
};

struct TransformComponent
{
    TransformComponent(vec3 pos, vec3 scale = {1,1,1}, quat rotation = {0,0,0,0})
        :pos(pos), scale(scale), rotation(rotation){}
    TransformComponent(){}
    mat4 transform()
    {
        return translate(mat4(1.0f), pos+offsetPos) * glm::scale(mat4(1.0f),scale)
               * glm::toMat4(rotation);
    }
    vec3 pos =       {0, 0, 0};
    vec3 scale =     {1, 1, 1};
    quat rotation =  {1,0,0,0};

    vec3 offsetPos = {0, 0, 0};
};

struct NormalDrawComponent
{
    // without this component or light component
    // render pipeline will ignore the entity
    // for now its only color, there will come more
    NormalDrawComponent(){}
    NormalDrawComponent(vec4 col)
        : color(col){}
    vec4 color = {0,0,0,0};
};

struct InstancedDrawComponent
{
    InstancedDrawComponent(){}
    InstancedDrawComponent(uint group)
        : instancedGroup(group){}
    uint instancedGroup = 0;
};

struct TerrainGenComponent
{
    // TODO: for now renderer will assign a heightmap here,
    //       later it should be created according to user spec
    TerrainGenComponent(){};

    float* heightMap = nullptr;
    bool terrainChanged = false;
    uint width  = 0;
    uint height = 0;
    float amplitude = 0;
};

struct EmissiveComponent
{
    EmissiveComponent(){}
    EmissiveComponent(vec4 color, float intensity)
        :color(color), emissiveIntensity(intensity){}
    vec3 color = {0,0,0};
    float emissiveIntensity = 1.0f;
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

struct AudioSourceComponent
{
    AudioSourceComponent(){}
    AudioSourceComponent(std::shared_ptr<AudioBuffer> buffer)
    {
        source = std::make_shared<AudioSource>(buffer);
    }

    std::shared_ptr<AudioSource> source;
};

struct CameraComponent
{

};

struct CameraControllerComponent
{

};


