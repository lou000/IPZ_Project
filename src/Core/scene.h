#pragma once
#include <memory>
#include <array>
#include "../Renderer/mesh.h"
#include "../Renderer/light.h"
#include "../Renderer/camera.h"
#include "../Renderer/shader.h"
#include "entity.h"

// For now this is base class that represents the "game"
class Scene
{
public:
    Scene(){};
    virtual ~Scene(){};
    virtual void onUpdate(float dt) = 0;
    virtual void onStart() = 0;

    Entity* getEntity();
    Entity* getEntity(std::vector<std::shared_ptr<Mesh>> meshes, vec3 pos = {0, 0, 0}, quat rotation = {1, 0, 0, 0});
    uint activeCount = 0;

    std::array<Entity, 100> entities;
    std::array<PointLight, 100> lights;
    DirectionalLight directionalLight;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Shader> pbrShader;


};

