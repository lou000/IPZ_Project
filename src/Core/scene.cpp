#include "scene.h"


Entity* Scene::getEntity()
{
    auto ent = &entities[activeCount];
    ent->enabled = true;
    activeCount++;
    return ent;
}

Entity *Scene::getEntity(std::shared_ptr<Model> model, vec3 pos, quat rotation)
{
    auto ent = &entities[activeCount];
    ent->enabled = true;
    ent->renderable = true;
    ent->model = model;
    ent->pos = pos;
    ent->rotation = rotation;
    activeCount++;
    return ent;
}
