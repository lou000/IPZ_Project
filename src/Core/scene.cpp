#include "scene.h"
#include "components.h"
#include "../Core/application.h"
#include "../Core/yamlserialization.h"
#include "../Core/gui.h"
#include "entity.h"
#include <memory>
#include <random>

//TODO: serialize only when prompted on exit and make a button
Scene::Scene(std::string name, bool deserialize)
    : m_name(name), m_deserialize(deserialize)
{
    //Add built in meshes
    AssetManager::addAsset(Model::makeUnitPlane());

    //Deserialize if enabled and succesful
//    if(m_deserialize && Serializer::deserializeScene(this, "../Config/"+m_name+".pc"))
//        return;

    //Default values in case there is no file or serialization is off
    auto winSize = App::getWindowSize();
    m_editorCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_gameCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);

    m_editorCamera->setFov(50.f);
    m_editorCamera->setPosition({0, 7, 20});
    m_editorCamera->setFocusPoint({0,6,0});

    m_gameCamera->setFov(50.f);
    m_gameCamera->setPosition({0, 7, 20});
    m_gameCamera->setFocusPoint({0,6,0});

    m_activeCamera = m_gameCamera;

    directionalLight.direction = normalize(vec3(-6, -5, -1.33f));
    directionalLight.color = {1,1,1};
    directionalLight.intensity = 1.f;
    directionalLight.ambientIntensity = 0.1f;
}

Scene::~Scene()
{
    Serializer::serializeScene(this, "../Config/"+m_name+".pc");
}

void Scene::sceneSettingsRender()
{
    if (ImGui::Begin("Scene settings", &showSceneSettings))
    {
        TWEAK_VEC3("Direction", directionalLight.direction, 0.01f, -10, 10);
        TWEAK_COLOR3("Color", directionalLight.color);
        TWEAK_FLOAT("Intensity", directionalLight.intensity, 0.01f, 0, 10);
        TWEAK_FLOAT("AmbientIntensity", directionalLight.ambientIntensity, 0.001f, 0, 1);
    }
    ImGui::End();
    onGuiRender();
}

void Scene::update(float dt)
{
    // Update sound listener
    m_activeCamera->onUpdate(dt);
    auto camPos = m_activeCamera->getPos();
//    LOG("Source pos: %f, %f, %f\n", camPos.x, camPos.y, camPos.z);
//    LOG("AL error: %d\n", alGetError());
    m_audioListener.setPosition(camPos);
    m_audioListener.setOrientation(m_activeCamera->forward(), m_activeCamera->up());

    // Update all sound sources that have transform components
    auto view = m_entities.view<TransformComponent, AudioSourceComponent>();
    for(auto& ent : view)
    {
        auto& pos = view.get<TransformComponent>(ent).pos;
//        LOG("Source pos: %f, %f, %f\n", pos.x, pos.y, pos.z);
        view.get<AudioSourceComponent>(ent).source->setPosition(pos);
    }
}

Entity Scene::createEntity(bool serialize)
{
    Entity entity = {m_entities.create(), this };
    if(!entity.hasComponent<IDComponent>())
    {
        auto& id = entity.addComponent<IDComponent>(genID());
        id.serialize = serialize;
    }
    else
    {
        WARN("What the fuck?");
        m_entities.destroy(entity);
        entity = { m_entities.create(), this };
        auto& id = entity.addComponent<IDComponent>(genID());
        id.serialize = serialize;
    }
    return entity;
}

Entity Scene::createEntity(const std::string &meshName, bool serialize, vec3 pos, vec3 scale, quat rotation, vec4 color)
{
    Entity entity = createEntity(serialize);
    entity.addComponent<TransformComponent>(pos, scale, rotation);
    entity.addComponent<MeshComponent>(meshName);
    entity.addComponent<NormalDrawComponent>(color);
    return entity;
}

Entity Scene::createNamedEntity(const std::string &entityName, const std::string &meshName, vec3 pos, vec3 scale, quat rotation, vec4 color)
{
    if(namedEntities.find(entityName) != namedEntities.end())
    {
        WARN("Entity %s already exists, overwriting!", entityName.c_str());
        auto ent = namedEntities.at(entityName);
        auto& transform = ent.getComponent<TransformComponent>();
        transform.pos = pos;
        transform.scale = scale;
        transform.rotation = rotation;
        auto& meshC = ent.getComponent<MeshComponent>();
        meshC.model = std::make_shared<Model>(meshName);
        auto& draw = ent.getComponent<NormalDrawComponent>();
        draw.color = color;
        return ent;
    }
    Entity entity = createEntity(true);
    namedEntities.emplace(entityName, entity);
    entity.addComponent<TagComponent>(entityName);
    entity.addComponent<TransformComponent>(pos, scale, rotation);
    entity.addComponent<MeshComponent>(meshName);
    entity.addComponent<NormalDrawComponent>(color);
    return entity;
}

Entity Scene::createInstanced(uint instancedGroup, const std::string &meshName, vec3 pos, vec3 scale, quat rotation)
{
    Entity entity = createEntity(false);
    entity.addComponent<TransformComponent>(pos, scale, rotation);
    entity.addComponent<MeshComponent>(meshName);
    entity.addComponent<InstancedDrawComponent>(instancedGroup);
    return entity;
}

Entity Scene::createPointLight(vec3 pos, bool serialize, vec3 color,
                               float intensity, float radius,
                               bool shadowCasting)
{
    Entity entity = createEntity(serialize);
    entity.addComponent<PointLightComponent>(pos, color, intensity, radius, shadowCasting);
    return entity;
}

Entity Scene::getEntity(const std::string &entityName)
{
    return namedEntities.at(entityName);
}

void Scene::removeEntity(Entity entity)
{
    m_entities.destroy(entity);
}

void Scene::swapCamera()
{
    if(m_activeCamera == m_gameCamera)
        m_activeCamera = m_editorCamera;
    else
        m_activeCamera = m_gameCamera;
}

Entity Scene::fromEntID(entt::entity id)
{
    ASSERT(m_entities.valid(id));
    return Entity(id, this);
}

void Scene::setGameCamera(std::shared_ptr<Camera> camera)
{
    if(m_activeCamera == m_gameCamera)
        m_activeCamera = camera;
    m_gameCamera = camera;
}

uint64 Scene::genID()
{
    static std::random_device randDevice;
    static std::mt19937_64 engine(randDevice());
    static std::uniform_int_distribution<uint64> uniformDist;

    return uniformDist(engine);
}
