#include "scene.h"


Entity* Scene::getEntity()
{
    auto ent = &entities[activeCount];
    ent->enabled = true;
    activeCount++;
    return ent;
}

Entity *Scene::getEntity(std::vector<std::shared_ptr<Mesh> > meshes, vec3 pos, quat rotation)
{
    auto ent = &entities[activeCount];
    ent->enabled = true;
    ent->renderable = true;
    ent->meshes = meshes;
    ent->pos = pos;
    ent->rotation = rotation;
    activeCount++;
    return ent;
}
