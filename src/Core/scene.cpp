#include "scene.h"


Entity* Scene::createEntity()
{
    auto ent = &entities[activeEntityCount++];
    ent->enabled = true;
    return ent;
}

PointLight *Scene::createLight(vec3 pos, vec3 color, float range, float intensity)
{
    auto light = &lights[activeLightCount++];
    light->enabled = true;
    light->pos = vec4(pos, 1);
    light->color = vec4(color, 1);
    light->range = range;
    light->intensity = intensity;
    return light;
}

Entity *Scene::createEntity(std::shared_ptr<Model> model, vec3 pos, vec3 scale, quat rotation)
{
    auto ent = &entities[activeEntityCount];
    ent->enabled = true;
    ent->renderable = true;
    ent->model = model;
    ent->pos = pos;
    ent->scale = scale;
    ent->rotation = rotation;
    activeEntityCount++;
    return ent;
}
