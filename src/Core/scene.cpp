#include "scene.h"
#include <memory>
#include "../Core/application.h"
#include "../Core/yamlserialization.h"


Scene::Scene(std::string name, bool serialize)
    : m_name(name), m_serialize(serialize)
{
    auto winSize = App::getWindowSize();
    m_editorCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_gameCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_activeCamera = m_editorCamera;
}

Scene::~Scene()
{
    if(m_serialize)
        serialize();
}

void Scene::serialize()
{
    //TODO: dont overwrite the file, derived class will write first
    Serializer::serializeScene(this, "../Config/"+m_name+".pc");
}

std::vector<std::shared_ptr<Entity>> Scene::enabledEntities()
{
    return entities;
}

std::vector<std::shared_ptr<PointLight>> Scene::enabledLights()
{
    return lights;
}

std::shared_ptr<PointLight> Scene::createLight(vec3 pos, vec3 color, float range, float intensity)
{
    auto light = std::make_shared<PointLight>();
    light->m_enabled = true;
    light->pos = vec4(pos, 1);
    light->color = vec4(color, 1);
    light->radius = range;
    light->intensity = intensity;
    lights.push_back(light);
    return light;
}

template<typename T>
std::shared_ptr<T> Scene::createEntity()
{
    auto ent = std::make_shared<T>();

    ent->m_enabled = true;
    entities.push_back(std::static_pointer_cast<Entity>(ent));
    return ent;
}

std::shared_ptr<Entity> Scene::createEntity(std::shared_ptr<Model> model, vec3 pos, vec3 scale, quat rotation)
{
    auto ent = std::make_shared<Entity>();
    ent->m_enabled = true;
    ent->renderable = true;
    ent->model = model;
    ent->pos = pos;
    ent->scale = scale;
    ent->rotation = rotation;
    entities.push_back(ent);
    return ent;
}
