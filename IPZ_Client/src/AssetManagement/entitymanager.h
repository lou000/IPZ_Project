#pragma once
#include <unordered_map>
#include <unordered_set>
#include "../Core/utilities.h"
#include "../Renderer/renderable.h"
#define MAX_ENTITIES 2000
// unordered_map<int, weak?_pointer>
// large preallocated memory block
// deleted entities marked as free memory
// free blocks of memory in std::set sorted by size
// we would have to check on insertion and deletion if we need to unfragment something
// or we can just reallocate everything when the deletions reach certain treshold
// lookup first element to see if we can allocate if not allocate at the end
// additional unordered_map<int, vector<ptr>> to quickly get entities by type?
// get weak_pointer when we retrieve entities?
// this might get complicated and not be very fast
using namespace glm;

class Entity
{
public:
    virtual void onCreate() = 0;
    virtual void onUpdate(float dt) = 0;
    virtual void onDestroy() = 0;

private:
    uint id;
    uint type;
    vec3 pos;
    quat rotation;
};

class EntityManager
{
public:
    EntityManager();
    std::weak_ptr<Entity> getEntity(uint id);
    std::weak_ptr<Entity> createEntity(uint type);
    void deleteEntity(uint id);
private:
    std::unordered_map<uint, std::weak_ptr<Entity>> entitiesByID;
    std::unordered_map<uint, std::weak_ptr<Entity>> entitiesByType;
    std::unordered_map<uint, std::shared_ptr<RenderSpec>> renderables;
    std::unordered_set<Entity*> availableMemory;
    Entity* entities;
};



