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
    friend class Serializer;
public:
    Scene();
    virtual ~Scene(){};
    virtual void onUpdate(float dt) = 0;
    virtual void onStart() = 0;
    virtual void debugDraw() = 0;

    DirectionalLight directionalLight;
    std::vector<std::shared_ptr<Entity>> enabledEntities();
    std::vector<std::shared_ptr<PointLight>> enabledLights();
    std::shared_ptr<Camera> activeCamera(){return m_activeCamera;}
    std::shared_ptr<Camera> sceneCamera(){return m_gameCamera;}
    std::shared_ptr<Camera> editorCamera(){return m_editorCamera;}

protected:
    std::shared_ptr<PointLight> createLight(vec3 pos, vec3 color, float range, float intensity);
    std::shared_ptr<Entity> createEntity(std::shared_ptr<Model> model, vec3 pos = {0, 0, 0}, vec3 scale = {1, 1, 1}, quat rotation = {1, 0, 0, 0});

    template<typename T>
    std::shared_ptr<T> createEntity();

private:
    std::shared_ptr<Camera> m_activeCamera;
    std::shared_ptr<Camera> m_gameCamera;
    std::shared_ptr<Camera> m_editorCamera;


    //TODO: Entity manager
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::shared_ptr<PointLight>> lights;
};

