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
    if(m_deserialize && Serializer::deserializeScene(this, "../Config/"+m_name+".pc"))
        return;

    //Default values in case there is no file or serialization is off
    auto winSize = App::getWindowSize();
    m_editorCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_gameCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    m_activeCamera = m_editorCamera;
    m_activeCamera->setFov(50.f);
    m_activeCamera->setPosition({0, 7, 20});
    m_activeCamera->setFocusPoint({0,6,0});

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
    m_audioListener.setPosition(m_activeCamera->getPos());
    m_audioListener.setOrientation(m_activeCamera->forward(), m_activeCamera->up());

    // Update all sound sources that have transform components
    auto view = m_entities.view<TransformComponent, AudioSourceComponent>();
    for(auto& ent : view)
    {
        auto& pos = view.get<TransformComponent>(ent).pos;
        view.get<AudioSourceComponent>(ent).source->setPosition(pos);
    }
}

Entity Scene::createEntity()
{
    Entity entity = { m_entities.create(), this };
    entity.addComponent<IDComponent>(genID());
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

Entity Scene::createInstanced(uint instancedGroup, const std::string &meshName, vec3 pos, vec3 scale, quat rotation)
{
    Entity entity = createEntity();
    entity.addComponent<TransformComponent>(pos, scale, rotation);
    entity.addComponent<MeshComponent>(meshName);
    entity.addComponent<RenderSpecComponent>(vec4(0,0,0,0));
    entity.addComponent<InstancedDrawComponent>(instancedGroup);
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

Entity Scene::fromEntID(entt::entity id)
{
    ASSERT(m_entities.valid(id));
    return Entity(id, this);
}

uint64 Scene::genID()
{
    static std::random_device randDevice;
    static std::mt19937_64 engine(randDevice());
    static std::uniform_int_distribution<uint64> uniformDist;

    return uniformDist(engine);
}
