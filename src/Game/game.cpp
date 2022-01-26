#include "game.h"
#include "../Core/entity.h"
#include "../Core/components.h"
#include "../Renderer/meshrenderer.h"

Game::Game() : Scene("TheGame", false)
{
    AssetManager::addAsset(MeshRenderer::createTriMeshGrid("terrain", 200, 200));
    if(!deserialized())
    {
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/obelisk1.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/wolf.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/campfire.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree5.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/torch2.fbx"));

        auto terrain = createNamedEntity("terrain", "terrain", vec3(-100,0,-100), vec3(1));
        auto& terrainGen = terrain.addComponent<TerrainGenComponent>();
        terrainGen.amplitude = 30;
        terrainGen.width = 200;
        terrainGen.height = 200;


        unsigned int amount = 2000;
        srand(glfwGetTime()); // initialize random seed
        float radius = 20.0;
        float offset = 80.f;
        for(unsigned int i = 0; i < amount; i++)
        {
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)amount * 360.0f;
            float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;
            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float z = cos(angle) * radius + displacement;

            // 2. scale: scale between 0.05 and 0.25f
            float scale = (rand() % 10) / 100.0f + 0.3;

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            float rotAngle = (rand() % 360);
            createInstanced(1, "../assets/meshes/tree5.fbx", vec3(x,0,z), vec3(scale), quat({-radians(90.f),radians(rotAngle),0}));
        }
    }
}

void Game::onStart()
{
    if(!deserialized())
    {
        auto terrain = getEntity("terrain");
        updateEntityHeightToTerrain(terrain);
    }
}

void Game::onUpdate(float dt)
{
    auto terrain = getEntity("terrain");
    auto& terrainMap = terrain.getComponent<TerrainGenComponent>();

    if(terrainMap.terrainChanged)
        updateEntityHeightToTerrain(terrain);
}

void Game::onDebugDraw()
{

}

void Game::onGuiRender()
{

}

void Game::updateEntityHeightToTerrain(Entity terrain)
{
    // snap all entities to terrain height at the start
    auto transform = terrain.getComponent<TransformComponent>();
    auto& terrainMap = terrain.getComponent<TerrainGenComponent>();
    if(terrainMap.heightMap)
    {
        auto group = entities().view<TransformComponent, MeshComponent>(entt::exclude<TerrainGenComponent>);
        for(auto& ent : group)
        {
            auto pos = group.get<TransformComponent>(ent).pos - transform.pos;
            // calculate x coords
            float x1 = glm::round(pos.x);

            // calculate z coords
            float y1 = glm::round(pos.z);

            int index = (int)(y1*terrainMap.width+(x1));
            float h1 = terrainMap.heightMap[index]*30-15;

            group.get<TransformComponent>(ent).offsetPos.y = h1;
        }
        terrainMap.terrainChanged = false;
    }
}


