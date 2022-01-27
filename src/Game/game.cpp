#include "game.h"
#include "../Core/entity.h"
#include "../Core/components.h"
#include "../Renderer/meshrenderer.h"
#include "../Core/yamlserialization.h"



Game::Game() : Scene("TheGame", true)
{
    AssetManager::addAsset(MeshRenderer::createTriMeshGrid("terrain", 200, 200));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/obelisk1.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/wolf.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/campfire.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree2.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree3.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree4.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree5.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/torch2.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/cube.obj"));

    Serializer::deserializeScene(this, "../Config/"+m_name+".pc");
    if(!deserialized())
    {
        auto terrain = createNamedEntity("terrain", "terrain", vec3(-100,0,-100), vec3(1));
        auto& terrainGen = terrain.addComponent<TerrainGenComponent>();
        terrainGen.amplitude = 30;
        terrainGen.width = 200;
        terrainGen.height = 200;
    }

    auto winSize = App::getWindowSize();
    gameCamera = std::make_shared<GameCamera>(50, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    gameCamera->setPosition(cameraOffset);
    gameCamera->pointAt({0,0,0});
    setGameCamera(gameCamera);

    auto player = createNamedEntity("player", "../assets/meshes/cube.obj");
    player.getComponent<NormalDrawComponent>().color = {2,2,2,1};
    player.getComponent<TransformComponent>().scale = {1, 2, 1};

    m_clearings = generateClearings({200, 200});
    m_paths = generatePaths(m_clearings);
    generateTrees();
    generateLanterns();


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

    if(App::getKeyOnce(GLFW_KEY_C))
        swapCamera();

    updatePlayer(dt, terrain);
}

void Game::updatePlayer(float dt, Entity terrain)
{
    auto player = getEntity("player");
    auto& transform = player.getComponent<TransformComponent>();

    if(App::getMouseButton(GLFW_MOUSE_BUTTON_LEFT))
    {
        playerC.moveTarget = mouseWorldPosition-transform.offsetPos;
        gameCamera->animateMove(cameraOffset+playerC.moveTarget);
        playerC.moving = true;
    }
    if(playerC.moving)
    {
        auto dir = playerC.moveTarget - transform.pos;
        auto step = normalize(dir)*playerC.speed*dt;

        if(length(step)>distance(transform.pos, playerC.moveTarget))
        {
            transform.pos = playerC.moveTarget;
            playerC.moving = false;
        }
        else
            transform.pos += step;


    }

//        auto terrainTransform = terrain.getComponent<TransformComponent>();
//        auto& terrainMap = terrain.getComponent<TerrainGenComponent>();
//        auto pos = transform.pos - terrainTransform.pos;
//        // calculate x coords
//        float x1 = glm::round(pos.x);

//        // calculate z coords
//        float y1 = glm::round(pos.z);

//        int index = (int)(y1*terrainMap.width+(x1));
//        if(index<512*512 && index>0)
//        {
//            float h1 = terrainMap.heightMap[index]*30-15;
//            playerC.heightTarget = h1;
//        }
}

void Game::onDebugDraw()
{

}

void Game::onGuiRender()
{

}

void Game::generateTrees()
{
    const char* treeMeshes[5] ={
        "../assets/meshes/tree.fbx",
        "../assets/meshes/tree2.fbx",
        "../assets/meshes/tree3.fbx",
        "../assets/meshes/tree4.fbx",
        "../assets/meshes/tree5.fbx"
    };

    for(uint i=0; i<8000; i++)
    {
        uint meshIndex = linearRand(0, 4);
        const char* randomTree = treeMeshes[meshIndex];
        float randomScale = linearRand(0.4f, 0.5f);
        float randomHeight = linearRand(0.6f, 0.8f);
        float randomRotation = linearRand(0.f, PI*2.f);
        vec2 pos = linearRand(vec2(-100.f, -100.f), vec2(100.f, 100.f));

        bool weGood = true;
        // check if inside one of circles
        for(auto c : m_clearings)
        {
            if(distance(c.pos, pos)<c.radius)
            {
                weGood = false;
                break;
            }
        }
        if(weGood)        //check paths
        {
            for(auto path : m_paths)
            {
                float dist = distancePointToLine(path.pointBegin, path.pointEnd, pos);
                if(dist<path.width)
                {
                    weGood = false;
                    break;
                }
            }
        }
        if(!weGood)
            i--;
        else
            createInstanced(meshIndex, randomTree, vec3(pos.x, 0, pos.y), vec3(randomScale, randomHeight, randomScale),
                            quat({-radians(90.f),randomRotation,0}));
    }
}

void Game::generateLanterns()
{
    for(auto path : m_paths)
    {
        auto dir = glm::normalize(path.pointEnd-path.pointBegin);
        auto distance = glm::distance(path.pointBegin, path.pointEnd);
        auto step = 20.f;
        uint count = distance/step;

        for(uint i=2; i<count; i++)
        {
            auto pos = path.pointBegin+(dir*step*(float)i);
            bool weGood = true;
            // check if inside one of circles
            for(auto c : m_clearings)
            {
                if(glm::distance(c.pos, pos)<c.radius)
                {
                    weGood = false;
                    break;
                }
            }
            glm::vec2 perp = glm::perp(glm::vec2(0, 1), dir);
            pos+=normalize(perp)*1.2f;
            if(weGood)
            {
                createInstanced(10, "../assets/meshes/torch2.fbx", vec3(pos.x, 0, pos.y), vec3(0.05), quat({-radians(90.f),0,0}));
                createPointLight({pos.x, 5, pos.y}, false, {1.f,0.6f,0.f}, 15.f, 20.f);
            }
        }
    }
}

void Game::updateEntityHeightToTerrain(Entity terrain)
{
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
            if(index<512*512 && index>0)
            {
                float h1 = terrainMap.heightMap[index]*30-15;
                group.get<TransformComponent>(ent).offsetPos.y = h1;
            }
        }
        terrainMap.terrainChanged = false;
    }
    auto view = entities().view<PointLightComponent>();
    for(auto &ent : view)
    {
        auto pos = view.get<PointLightComponent>(ent).light.pos - vec4(transform.pos, 0);
        // calculate x coords
        float x1 = glm::round(pos.x);

        // calculate z coords
        float y1 = glm::round(pos.z);

        int index = (int)(y1*terrainMap.width+(x1));
        float h1 = terrainMap.heightMap[index]*30-15;

        view.get<PointLightComponent>(ent).light.pos.y = h1+2; //add offset to pointlight
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

    for(uint i=0; i<15; i++)
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
    std::vector<Path> out;

    //we pass clearings by value so we can delete them here
    ASSERT(clearings.size()>1);

    auto first = clearings[0];
    clearings.erase(clearings.begin());
    for(uint i=0; i<clearings.size();)
    {
        auto randomIndex = linearRand(0ull, clearings.size()-1);
        auto second = clearings[randomIndex];
        Path p;
        p.pointBegin = first.pos;
        p.pointEnd = second.pos;
        p.width = 2;
        out.push_back(p);
        clearings.erase(clearings.begin()+randomIndex);
        first = second;
    }

    return out;
}


