#include "scene.h"
#include "components.h"
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

Entity Scene::createEntity()
{
    Entity entity = { m_entities.create(), this };
    entity.m_id = genID();
    return entity;
}

Entity Scene::createEntity(const std::string &meshName, vec3 pos, vec3 scale, quat rotation, vec4 color)
{
    Entity entity = createEntity();
    entity.addComponent<TransformComponent>(pos, scale, rotation);
    entity.addComponent<MeshComponent>(meshName);
    entity.addComponent<RenderSpecComponent>(color);
    return entity;
}

Entity Scene::createPointLight(vec3 pos, vec3 color,
                               float intensity, float radius,
                               bool shadowCasting)
{
    Entity entity = createEntity();
    entity.addComponent<PointLightComponent>(pos, color, intensity, radius, shadowCasting);
    return entity;
}

void Scene::removeEntity(Entity entity)
{
    m_entities.destroy(entity);
}

uint64 Scene::genID()
{
    static std::random_device randDevice;
    static std::mt19937_64 engine(randDevice());
    static std::uniform_int_distribution<uint64> uniformDist;

    return uniformDist(engine);
}
