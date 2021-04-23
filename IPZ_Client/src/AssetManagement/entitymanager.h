#pragma once
#include <unordered_map>
#include <unordered_set>
#include "../Core/utilities.h"
#include "../Renderer/renderspec.h"
#define MAX_ENTITIES 2000
using namespace glm;

uint64 idCounter = 0; //i have no idea if this will work, but by the time this goes around the entity should be cleaned up

//Im extremely not sure about this prototype
class Entity
{
public:
    virtual void onCreate() = 0;
    virtual void onUpdate(float dt) = 0;
    virtual void onDestroy() = 0;
    virtual void draw() = 0;

private:
    //maybe add here some functions from Camera
    uint64 id;
    uint type;
    vec3 pos;
    quat rotation;
    std::shared_ptr<RenderSpec> renderSpec;
    std::shared_ptr<Asset> asset;
};

class EntityManager
{
public:
    EntityManager();
    std::weak_ptr<Entity> getEntity(uint id);
    std::weak_ptr<Entity> createEntity(uint type);
    void deleteEntity(uint id);
private:
    std::unordered_map<uint, std::shared_ptr<Entity>> entitiesByID;
    std::unordered_map<uint, std::vector<std::shared_ptr<Entity>>> entitiesByType;
    std::unordered_map<std::shared_ptr<RenderSpec>, std::vector<std::shared_ptr<Entity>>> entitiesByRenderSpec;
    std::unordered_set<std::shared_ptr<Entity>> availableMemory;
    Entity* entities;
};



