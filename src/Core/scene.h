#pragma once
#include <memory>
#include <array>
#include "../Renderer/mesh.h"
#include "../Renderer/light.h"
#include "../Renderer/camera.h"
#include "../Renderer/shader.h"
#include "entity.h"
#include <unordered_set>

#define MAX_LIGHTS 100

// For now this is base class that represents the "game"
class Scene
{
    friend class Serializer;
public:
    Scene(std::string name, bool serialize = true);
    Scene(const Scene&) = delete;
    virtual ~Scene();
    virtual void onUpdate(float dt) = 0;
    virtual void onStart() = 0;
    virtual void debugDraw() = 0;

    DirectionalLight directionalLight; //serialized
    std::unordered_set<std::shared_ptr<Entity>> entities();
    std::shared_ptr<Camera> activeCamera(){return m_activeCamera;}
    std::shared_ptr<Camera> sceneCamera(){return m_gameCamera;}
    std::shared_ptr<Camera> editorCamera(){return m_editorCamera;}

protected:
    void registerEntity(std::shared_ptr<Entity> entity);
    void removeEntity(std::shared_ptr<Entity> entity);

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


    // these are serialized by the inheriting classes
    std::unordered_set<std::shared_ptr<Entity>> m_entities;
};

