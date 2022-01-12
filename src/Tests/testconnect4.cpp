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
    :Scene("testConnect4", true)
{
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/connect4_board.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/connect4Puck1.obj"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/connect4Puck2.obj"));

    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/obelisk1.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/wolf.fbx"));
    AssetManager::addAsset(std::make_shared<Model>("../assets/meshes/campfire.fbx"));
    AssetManager::addAsset(Model::makeUnitPlane());



    auto board = std::make_shared<Board>();

    registerEntity(board);

    registerEntity(std::make_shared<Decoration>("../assets/meshes/obelisk1.fbx", vec3(5,0,5),
                                                vec3(1), quat({-radians(90.f), 0, 0})));
    registerEntity(std::make_shared<Decoration>("../assets/meshes/wolf.fbx", vec3(-5,0,5),
                                                vec3(1),  quat({-radians(90.f), 0, 0})));
    registerEntity(std::make_shared<Decoration>("../assets/meshes/campfire.fbx", vec3(1,0,3),
                                                vec3(0.3f), quat({-radians(90.f), 0, 0})));
    registerEntity(std::make_shared<Decoration>("unitPlane", vec3(0,0,0), vec3(100)));
    registerEntity(std::make_shared<PointLight>(vec3(1, 2 ,3), vec4(1,0.05,0,1), 60.f, 200.f));


    previewPuck = std::make_shared<Puck>(Puck::Yellow);
    previewPuck->color.a = 0.2f;
    registerEntity(previewPuck);



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

void TestConnect4::debugDraw()
{
    for(auto ent : entities())
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

Board::Board()
    : Entity(Entity::C4Board)
{
    model = AssetManager::getAsset<Model>("../assets/meshes/connect4_board.fbx");
    renderable = true;
}

Puck::Puck(Color col)
    : Entity(Entity::C4Puck)
{
    switch(col)
    {
        case Yellow:
            model = AssetManager::getAsset<Model>("../assets/meshes/connect4Puck1.obj");
            color = {0.906, 0.878, 0.302, 1};
            break;
        case Red:
            model = AssetManager::getAsset<Model>("../assets/meshes/connect4Puck2.obj");
            color = {0.882, 0.192, 0.161, 1};
            break;
    }
    rotation = quat({radians(90.f), 0, 0});
    boardPos = {-1, -1};
    renderable = true;
}

Decoration::Decoration(const std::string &meshName, vec3 pos, vec3 scale, quat rotation)
    : Entity(Entity::Decoration)
{
    this->model = AssetManager::getAsset<Model>(meshName);
    this->pos = pos;
    this->scale = scale;
    this->rotation = rotation;
    renderable = true;
}
