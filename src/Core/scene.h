#pragma once
#include <memory>
#include <array>
#include "../Renderer/mesh.h"
#include "../Renderer/light.h"
#include "../Renderer/camera.h"
#include "../Renderer/shader.h"
#include "entity.h"

#define MAX_LIGHTS 100

// For now this is base class that represents the "game"
class Scene
{
public:
    Scene(){};
    virtual ~Scene(){};
    virtual void onUpdate(float dt) = 0;
    virtual void onStart() = 0;
    virtual void debugDraw() = 0;

    std::shared_ptr<Camera> activeCamera;

    std::shared_ptr<Camera> camera;
    std::shared_ptr<Camera> editorCamera;
    std::shared_ptr<Shader> pbrShader;

    //TODO: Entity manager
    uint activeEntityCount = 0;
    uint activeLightCount = 0;
    Entity* createEntity();
    PointLight* createLight(vec3 pos, vec3 color, float range, float intensity);
    Entity* createEntity(std::shared_ptr<Model> model, vec3 pos = {0, 0, 0}, quat rotation = {1, 0, 0, 0});
    std::array<Entity, 100> entities;
    std::array<PointLight, MAX_LIGHTS> lights;
    DirectionalLight skyLight;

};

