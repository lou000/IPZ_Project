#pragma once
#include <memory>
#include <array>
#include "../Renderer/camera.h"
#include "entity.h"
#include "../Renderer/light.h"
#include <unordered_set>

#define MAX_LIGHTS 100

// For now this is base class that represents the "game"
class Scene
{
    friend class Entity;
    friend class Serializer;
public:
    Scene(std::string name, bool serialize = true);
    Scene(const Scene&) = delete;
    virtual ~Scene();
    virtual void onUpdate(float dt) = 0;
    virtual void onStart() = 0;
    virtual void debugDraw() = 0;

    DirectionalLight directionalLight; //serialized
    bool deserialized() {return m_deserialized;}
    entt::registry& entities(){return m_entities;}
    std::shared_ptr<Camera> activeCamera(){return m_activeCamera;}
    std::shared_ptr<Camera> sceneCamera(){return m_gameCamera;}
    std::shared_ptr<Camera> editorCamera(){return m_editorCamera;}

protected:
    Entity createEntity();
    Entity createEntity(const std::string &meshName, vec3 pos = {0,0,0},
                        vec3 scale = {1,1,1}, quat rotation = {1, 0, 0, 0},
                        vec4 color = {0,0,0,0});
    Entity createPointLight(vec3 pos, vec3 color = {1,1,1}, float intensity = 1.0f, float radius = 10.f, bool shadowCasting = false);

    void removeEntity(Entity entity);

private:
    uint64 genID();
    void addDeserializedEntity(std::shared_ptr<Entity> entity, uint64 id);

    //             SERIALIZED            //
    //-----------------------------------//
    std::shared_ptr<Camera> m_activeCamera;
    std::shared_ptr<Camera> m_gameCamera;
    std::shared_ptr<Camera> m_editorCamera;
    std::string m_name;
    bool m_serialize;
    //-----------------------------------//

    bool m_deserialized = false;
    entt::registry m_entities;
};

