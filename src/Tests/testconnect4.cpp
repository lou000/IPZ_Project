#include "testconnect4.h"
#include <sstream>
#include "../Core/math.h"
#include "../Core/entity.h"
#include "../Core/components.h"
#include "../Renderer/meshrenderer.h"

Move pickRandomTopMove(std::vector<std::pair<Move, double>> moves) //input sorted
{
    std::pair<Move, double> bestMove = moves.front();
    std::vector<std::pair<Move, double>> bestMoves;
    for(size_t i=0; i<moves.size(); i++)
    {
        auto move = moves[i];
        if(move.second>bestMove.second)
            bestMove = move;
    }
    std::wstringstream stream;
    stream<<"Computer moves: ";
    if(std::isinf(bestMove.second))
        return bestMove.first;

    for(auto move:moves)
    {
        if(move.second>=bestMove.second-abs(bestMove.second*0.05f))
        {
            stream<<"!!("<<move.first.x<<","<<move.first.y<<") g:"<<move.second<<"!! ";
            bestMoves.push_back(move);
        }
        else
            stream<<"  ("<<move.first.x<<","<<move.first.y<<") g:"<<move.second<<"   ";
    }
    std::wcout<<stream.str()<<"\n";
    int random = (int) glm::linearRand<int>(0, bestMoves.size()-1);
    ASSERT(bestMoves.size());
    return bestMoves[random].first;
}

TestConnect4::TestConnect4()
    :Scene("testConnect4", false)
{
    AssetManager::addAsset(MeshRenderer::createTriMeshGrid("terrain", 200, 200));
    if(!deserialized())
    {
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/connect4_board.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/connect4Puck1.obj"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/connect4Puck2.obj"));

        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/obelisk1.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/wolf.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/campfire.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/tree5.fbx"));
        AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/torch2.fbx"));

        AssetManager::addAsset(std::make_shared<AudioBuffer>("../assets/sounds/steps2.wav"));

        directionalLight.direction = {-0.757f, -0.631f, -0.978f};
        directionalLight.color = {0.979f, 0.931f, 0.736f};
        directionalLight.intensity = 1;

        auto terrain = createNamedEntity("terrain", "terrain", vec3(-100,0,-100), vec3(1));
        terrain.addComponent<TerrainGenComponent>();
        terrain.getComponent<TransformComponent>();

        createEntity("../assets/meshes/connect4_board.fbx");
        createEntity("../assets/meshes/obelisk1.fbx", vec3(5,0,5),
                     vec3(1), quat({-radians(90.f), 0, 0}), vec4(vec3(0.3),1));
        createEntity("../assets/meshes/wolf.fbx", vec3(-5,0,5),
                      vec3(1),  quat({-radians(90.f), 0, 0}));
        createEntity("../assets/meshes/torch2.fbx", vec3(1,0,-10),
                     vec3(0.3f), quat({-radians(90.f), 0, 0}));
        createEntity("../assets/meshes/campfire.fbx", vec3(1,0,3),
                     vec3(0.3f), quat({-radians(90.f), 0, 0}));


        createPointLight(vec3(1, 2 ,3), vec3(1,0.05,0), 200.f, 10.f);

//        auto soundTest = createEntity();
//        soundTest.addComponent<TransformComponent>(vec3(0,0,0));
//        auto src = soundTest.addComponent<AudioSourceComponent>(AssetManager::getAsset<AudioBuffer>("../assets/sounds/steps2.wav"));
//        src.source->play();
//        src.source->setLoop(true);

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



//    for(int i=0;i<7; i++)
//        hPositions[i] = leftSlot + i*hOffset;
//    for(int i=0;i<6; i++)
//        vPositions[i] = top      - i*vOffset;

//    c4 = new Connect4(7, 6);
//    searcher = new alpha_beta_searcher<Move, true>(3,true);

}

void TestConnect4::onStart()
{

}

void TestConnect4::onUpdate(float dt)
{
//    auto terrain = getEntity("terrain");
//    // TODO: mouseposition using compute based on depth and uv
//    auto transform = terrain.getComponent<TransformComponent>();
//    auto& terrainMap = terrain.getComponent<TerrainGenComponent>();

//    if(terrainMap.heightMap)
//    {
//        auto group = entities().view<TransformComponent, MeshComponent>(entt::exclude<TerrainGenComponent>);
////        for(int y=511; y>0; y--)
////            for(int x=0; x<200; x++)
////                LOG("(%d, %d): %f\n", x, y, terrainMap.heightMap[y*512+x]);
//        for(auto& ent : group)
//        {
//            auto pos = group.get<TransformComponent>(ent).pos - transform.pos;
//            // calculate x coords
//            float x1 = glm::round(pos.x);

//            // calculate z coords
//            float y1 = glm::round(pos.z);

//            int index = (int)(y1*terrainMap.width+(x1));
//            float h1 = terrainMap.heightMap[index]*30-15;

////            group.get<TransformComponent>(ent).pos.y = h1;
//        }
//        terrainMap.terrainChanged = false;

//    }





//    auto mouseRay  = activeCamera()->getMouseRay();
//    auto cameraPos = activeCamera()->getPos();
//    vec3 intersection = {};

//    previewPuck->renderable = false;
//    if(!App::getMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) && !animating && c4->userTurn && c4->is_terminal() == std::nullopt)
//    {
//        if(intersectPlane({0, 0, -1}, {0,0,0.45}, cameraPos, mouseRay, intersection))
//        {
//            for(int i=0; i<7; i++)
//                if(abs(intersection.x - hPositions[i])<=hOffset/2 && intersection.y>0 && intersection.y<10)
//                {
//                    currentMove = c4->createMove(i);
//                    if(currentMove.h_grade>=0)
//                    {
//                        if(App::getMouseButton(GLFW_MOUSE_BUTTON_LEFT))
//                        {
//                            puckInPlay = createEntity(mesh3, {hPositions[i],11,-0.45}, vec3(1), quat({radians(90.f), 0, 0}));
//                            puckInPlay->color = yellow;
//                            animating = true;
//                        }
//                        else
//                        {
//                            previewPuck->renderable = true;
//                            previewPuck->pos = {hPositions[i],11,-0.45};
//                        }
//                    }
//                }
//        }
//    }
//    if(!animating && !c4->userTurn && c4->is_terminal() == std::nullopt)
//    {
//        searcher->do_search(*c4);
//        auto moves = searcher->get_scores();
//        currentMove = pickRandomTopMove(moves);

//        puckInPlay = createEntity(mesh3, {0,0,0}, vec3(1), quat({radians(90.f), 0, 0}));
//        puckInPlay->color = red;

//        animating = true;
//    }

//    if(animating)
//    {
//        ASSERT(puckInPlay);
//        ySpeed += 20*dt;
//        yPos -= ySpeed*dt;

//        puckInPlay->pos = {hPositions[currentMove.x], yPos,-0.45};

//        if(yPos<=vPositions[currentMove.y])
//        {
//            animating = false;
//            puckInPlay->pos.y = vPositions[currentMove.y];
//            puckInPlay = nullptr;
//            ySpeed = 0;
//            yPos = 11;
//            c4->commitMove(currentMove);
//        }
//    }

}

void TestConnect4::onGuiRender()
{
}

void TestConnect4::onDebugDraw()
{
    auto view = entities().view<MeshComponent, TransformComponent>();
    for(auto ent : view)
    {
        auto model = view.get<TransformComponent>(ent).transform();
        auto bb = view.get<MeshComponent>(ent).model->boundingBox();
        bb.max = model*vec4(bb.max, 1);
        bb.min = model*vec4(bb.min, 1);
        BatchRenderer::drawLine(bb.min, {bb.max.x, bb.min.y, bb.min.z}, 0.05f, {1,0,0,1});
        BatchRenderer::drawLine(bb.min, {bb.min.x, bb.max.y, bb.min.z}, 0.05f, {1,0,0,1});
        BatchRenderer::drawLine(bb.min, {bb.min.x, bb.min.y, bb.max.z}, 0.05f, {1,0,0,1});

        BatchRenderer::drawLine(bb.max, {bb.min.x, bb.max.y, bb.max.z}, 0.05f, {1,0,0,1});
        BatchRenderer::drawLine(bb.max, {bb.max.x, bb.min.y, bb.max.z}, 0.05f, {1,0,0,1});
        BatchRenderer::drawLine(bb.max, {bb.max.x, bb.max.y, bb.min.z}, 0.05f, {1,0,0,1});

        BatchRenderer::drawLine({bb.min.x, bb.max.y, bb.min.z}, {bb.min.x, bb.max.y, bb.max.z}, 0.05f, {1,0,0,1});
        BatchRenderer::drawLine({bb.min.x, bb.max.y, bb.min.z}, {bb.max.x, bb.max.y, bb.min.z}, 0.05f, {1,0,0,1});

        BatchRenderer::drawLine({bb.max.x, bb.min.y, bb.max.z}, {bb.max.x, bb.min.y, bb.min.z}, 0.05f, {1,0,0,1});
        BatchRenderer::drawLine({bb.max.x, bb.min.y, bb.max.z}, {bb.min.x, bb.min.y, bb.max.z}, 0.05f, {1,0,0,1});

        BatchRenderer::drawLine({bb.max.x, bb.min.y, bb.min.z}, {bb.max.x, bb.max.y, bb.min.z}, 0.05f, {1,0,0,1});

        BatchRenderer::drawLine({bb.min.x, bb.min.y, bb.max.z}, {bb.min.x, bb.max.y, bb.max.z}, 0.05f, {1,0,0,1});
    }

}

