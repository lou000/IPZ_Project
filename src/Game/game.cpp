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


        auto clearings = generateClearings({200, 200});

        for(auto c : clearings)
        {
            for(uint i=0; i<50; i++)
            {
                auto randAngle = linearRand(0.f, PI*2.f);
                auto pos = vec2(glm::cos(randAngle)*c.radius, glm::sin(randAngle)*c.radius);
                createInstanced(1, "../assets/meshes/tree5.fbx", vec3(c.pos.x+pos.x, 0,c.pos.y+pos.y), vec3(0.3), quat({-radians(90.f),0,0}));
            }
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

std::vector<Clearing> Game::generateClearings(vec2 terrainSize)
{
    //generate first clearing
    std::vector<Clearing> out;
    Clearing first;
    float border = 20.f;
    float radiusMin = 5.f;
    float radiusMax = 15.f;
    vec2 maxPos = terrainSize/2.f - vec2(border);
    vec2 minPos = (terrainSize/2.f - terrainSize) + vec2(border);
    first.pos = {0,0};
    first.radius = linearRand(radiusMin, radiusMax);
    out.push_back(first);

    for(uint i=0; i<25; i++)
    {
        bool positionOK = false;
        Clearing c;
        int fuck = 0;
        while(!positionOK && fuck<=1000)
        {
            // random distance and direction that is not in prev clearing or outside of bounds
            c.pos = linearRand(minPos, maxPos);
            c.radius = linearRand(radiusMin, radiusMax);
            positionOK = true;
            for(auto clearing : out)
            {
                if(glm::distance(c.pos, clearing.pos) < c.radius+clearing.radius+5)
                {
                    positionOK = false;
                    break;
                }
            }
            fuck++;
            if(fuck>1000)
                LOG("Too many clearings man!");
        }
        if(fuck<=1000)
            out.push_back(c);
    }
    return out;
}

std::vector<Path> Game::generatePaths(std::vector<Clearing> clearings)
{
    // try to randomize the connections so we get intersections, we have to avoid intersecting with clearings with weird angle
    // maybe do frustum culling, the dumb way
    return {};
}


