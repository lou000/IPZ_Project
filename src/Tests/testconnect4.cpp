#include "testconnect4.h"
#include <sstream>
#include "../Core/math.h"

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
{
    mesh1 = std::make_shared<Model>("../assets/meshes/connect4_board.fbx");
    mesh2 = std::make_shared<Model>("../assets/meshes/connect4Puck1.obj");
    mesh3 = std::make_shared<Model>("../assets/meshes/connect4Puck2.obj");

    auto testMesh1 = std::make_shared<Model>("../assets/meshes/obelisk1.fbx");
    auto testMesh2 = std::make_shared<Model>("../assets/meshes/wolf.fbx");
    auto testMesh3 = std::make_shared<Model>("../assets/meshes/campfire.fbx");
    auto testMesh4 = Model::makeUnitQuad();
//    auto testMesh3 = std::make_shared<MeshFile>("../assets/meshes/tree.blend");

    AssetManager::addAsset(mesh1);
    AssetManager::addAsset(mesh2);
    AssetManager::addAsset(mesh3);

    AssetManager::addAsset(testMesh1);
    AssetManager::addAsset(testMesh2);
    AssetManager::addAsset(testMesh3);

    activeCamera()->setFov(50.f);
    activeCamera()->setPosition({0, 7, 20});
    activeCamera()->setFocusPoint({0,6,0});

    auto board = createEntity(mesh1, {0,0,0});
    board->color = {0.165, 0.349, 1.000, 1};

    auto puck1 = createEntity(mesh2, {1,0,3});
    puck1->color = red;

    auto puck2 = createEntity(mesh3, {-1,0,3});
    puck2->color = yellow;

    auto test1 = createEntity(testMesh1, {5,0,5}, vec3(1), quat({-radians(90.f), 0, 0}));
    auto test2 = createEntity(testMesh2, {-5,0,5}, vec3(1),  quat({-radians(90.f), 0, 0}));
    auto test3 = createEntity(testMesh3, {1,0,3}, vec3(0.3), quat({-radians(90.f), 0, 0}));
    auto test4 = createEntity(testMesh4, {0,0,0}, vec3(100));

    createLight({1, 2 ,3}, {1,0.05,0}, 60.f, 200.f);

    previewPuck = createEntity(mesh3, {0, 0, 0}, vec3(1), quat({radians(90.f), 0, 0}));
    previewPuck->color = yellow;
    previewPuck->color.a = 0.2f;

    directionalLight.direction = normalize(vec3(-6, -5, -1.33f));
    directionalLight.color = {1,1,1};
    directionalLight.intensity = 1.f;

    for(int i=0;i<7; i++)
        hPositions[i] = leftSlot + i*hOffset;
    for(int i=0;i<6; i++)
        vPositions[i] = top      - i*vOffset;

    c4 = new Connect4(7, 6);
    searcher = new alpha_beta_searcher<Move, true>(3,true);

}

void TestConnect4::onStart()
{

}

void TestConnect4::onUpdate(float dt)
{
    auto mouseRay  = activeCamera()->getMouseRay();
    auto cameraPos = activeCamera()->getPos();
    vec3 intersection = {};

    previewPuck->renderable = false;
    if(!App::getMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) && !animating && c4->userTurn && c4->is_terminal() == std::nullopt)
    {
        if(intersectPlane({0, 0, -1}, {0,0,0.45}, cameraPos, mouseRay, intersection))
        {
            for(int i=0; i<7; i++)
                if(abs(intersection.x - hPositions[i])<=hOffset/2 && intersection.y>0 && intersection.y<10)
                {
                    currentMove = c4->createMove(i);
                    if(currentMove.h_grade>=0)
                    {
                        if(App::getMouseButton(GLFW_MOUSE_BUTTON_LEFT))
                        {
                            puckInPlay = createEntity(mesh3, {hPositions[i],11,-0.45}, vec3(1), quat({radians(90.f), 0, 0}));
                            puckInPlay->color = yellow;
                            animating = true;
                        }
                        else
                        {
                            previewPuck->renderable = true;
                            previewPuck->pos = {hPositions[i],11,-0.45};
                        }
                    }
                }
        }
    }
    if(!animating && !c4->userTurn && c4->is_terminal() == std::nullopt)
    {
        searcher->do_search(*c4);
        auto moves = searcher->get_scores();
        currentMove = pickRandomTopMove(moves);

        puckInPlay = createEntity(mesh3, {0,0,0}, vec3(1), quat({radians(90.f), 0, 0}));
        puckInPlay->color = red;

        animating = true;
    }

    if(animating)
    {
        ASSERT(puckInPlay);
        ySpeed += 20*dt;
        yPos -= ySpeed*dt;

        puckInPlay->pos = {hPositions[currentMove.x], yPos,-0.45};

        if(yPos<=vPositions[currentMove.y])
        {
            animating = false;
            puckInPlay->pos.y = vPositions[currentMove.y];
            puckInPlay = nullptr;
            ySpeed = 0;
            yPos = 11;
            c4->commitMove(currentMove);
        }
    }

}

void TestConnect4::debugDraw()
{
    for(auto ent : enabledEntities())
    {
        auto model = ent->getModelMatrix();
        auto bb = ent->model->boundingBox();
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
