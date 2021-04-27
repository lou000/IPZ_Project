#include "entitymanager.h"

EntityManager::EntityManager()
{
    entities = (Entity*)malloc(MAX_ENTITIES*sizeof(Entity));
    auto freePtr = entities;
    for(int i=0; i<MAX_ENTITIES; i++)
    {
        availableMemory.insert(std::shared_ptr<Entity>(freePtr));
        freePtr++;
    }

}

std::weak_ptr<Entity> EntityManager::getEntity(uint id)
{
    return entitiesByID[id];
}

std::weak_ptr<Entity> EntityManager::createEntity(uint type)
{
    // memset to zero
    // add to all
    idCounter++;
    return std::weak_ptr<Entity>();
}

void EntityManager::deleteEntity(uint id)
{

}
