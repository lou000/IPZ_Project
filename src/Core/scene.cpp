#include "scene.h"
#include "../Core/application.h"
#include "../Core/yamlserialization.h"
#include <memory>
#include <random>


Scene::Scene(std::string name, bool serialize)
    : m_name(name), m_serialize(serialize)
{
    //Deserialize if enabled and succesful
    if(m_serialize && Serializer::deserializeScene(this, "../Config/"+m_name+".pc"))
        return;

    //Default values in case there is no file or serialization is off
    auto winSize = App::getWindowSize();
    m_editorCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_gameCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_activeCamera = m_editorCamera;

    directionalLight.direction = normalize(vec3(-6, -5, -1.33f));
    directionalLight.color = {1,1,1};
    directionalLight.intensity = 1.f;
}

Scene::~Scene()
{
    if(m_serialize)
        Serializer::serializeScene(this, "../Config/"+m_name+".pc");
}

std::unordered_set<std::shared_ptr<Entity>> Scene::entities()
{
    return m_entities;
}

uint64 Scene::genID()
{
    static std::random_device randDevice;
    static std::mt19937_64 engine(randDevice());
    static std::uniform_int_distribution<uint64> uniformDist;

    return uniformDist(engine);
}

void Scene::addDeserializedEntity(std::shared_ptr<Entity> entity, uint64 id)
{
    entity->m_id = id;
    m_entities.insert(entity);
}

void Scene::registerEntity(std::shared_ptr<Entity> entity)
{
    entity->m_id = genID();
    m_entities.insert(entity);
}

void Scene::removeEntity(std::shared_ptr<Entity> entity)
{
    m_entities.erase(entity);
}
