﻿#include "testconnect4.h"
#include <sstream>
bool intersectPlane(const vec3 &planeNormal, const vec3 &planePos, const vec3 &rayStartPos, const vec3 &ray, vec3 &intersection)
{
    // assuming vectors are all normalized
    float denom = dot(planeNormal, ray);
    if (denom > 1e-6) {
        vec3 p0l0 = planePos - rayStartPos;
        float t = dot(p0l0, planeNormal) / denom;
        if(t>=0)
        {
            intersection = rayStartPos + t*ray;
            return true;
        }
        else
            return false;
    }
    return false;
}
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
    int random = rndInt(0, bestMoves.size());
    ASSERT(bestMoves.size());
    return bestMoves[random].first;
}
TestConnect4::TestConnect4()
{
    mesh1 = std::make_shared<MeshFile>("../assets/meshes/connect4Board.obj");
    mesh2 = std::make_shared<MeshFile>("../assets/meshes/connect4Puck1.obj");
    mesh3 = std::make_shared<MeshFile>("../assets/meshes/connect4Puck2.obj");

    AssetManager::addAsset(mesh1);
    AssetManager::addAsset(mesh2);
    AssetManager::addAsset(mesh3);

    auto winSize = App::getWindowSize();
    camera = std::make_shared<Camera>(40.f, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    camera->setPosition({0, 7, 20});
    camera->setFocusPoint({0,6,0});
    camera->pointAt({0,6,0});

    GraphicsContext::setCamera(camera);
    BatchRenderer::setShader(AssetManager::getShader("batch"));
    MeshRenderer::setShader(AssetManager::getShader("mesh"));



    for(int i=0;i<7; i++)
        hPositions[i] = leftSlot + i*hOffset;
    for(int i=0;i<6; i++)
        vPositions[i] = top      - i*vOffset;

    c4 = new Connect4(7, 6);
    searcher = new alpha_beta_searcher<Move, true>(3,true);

}

void TestConnect4::onUpdate(float dt)
{
    auto mouseRay  = camera->getMouseRay();
    auto cameraPos = camera->getPos();
    vec3 intersection = {};

    MeshRenderer::begin();
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
                        auto model = translate(mat4(1.0f), {hPositions[i],11,-0.45}) * rotate(mat4(1.0f), radians(90.f), { 1.0f, 0.0f, 0.0f });
                        MeshRenderer::drawMesh(model, mesh3->mesh(), {0.906, 0.878, 0.302,0.2});
                        if(App::getMouseButton(GLFW_MOUSE_BUTTON_LEFT))
                            animating = true;
                    }
                }
        }
    }
    if(!animating && !c4->userTurn && c4->is_terminal() == std::nullopt)
    {
        searcher->do_search(*c4);
        auto moves = searcher->get_scores();
        currentMove = pickRandomTopMove(moves);
        animating = true;
    }

    if(animating)
    {
        ySpeed += 20*dt;
        yPos -= ySpeed*dt;
        auto model = translate(mat4(1.0f), {hPositions[currentMove.x], yPos,-0.45})*rotate(mat4(1.0f), radians(90.f), { 1.0f, 0.0f, 0.0f });
        if(c4->userTurn)
            MeshRenderer::drawMesh(model, mesh3->mesh(), {0.906, 0.878, 0.302,1});
        else
            MeshRenderer::drawMesh(model, mesh2->mesh(), {0.882, 0.192, 0.161,1});

        if(yPos<=vPositions[currentMove.y])
        {
            animating = false;
            ySpeed = 0;
            yPos = 11;
            c4->commitMove(currentMove);
        }
    }

    for(uint i=0; i<c4->size; i++)
    {
        uint x = i%c4->width;
        uint y = i/c4->width;
        auto model = translate(mat4(1.0f), {hPositions[x], vPositions[y],-0.45})*rotate(mat4(1.0f), radians(90.f), { 1.0f, 0.0f, 0.0f });
        if(c4->grid[i] == 'O')
            MeshRenderer::drawMesh(model, mesh3->mesh(), {0.906, 0.878, 0.302,1});
        else if(c4->grid[i] == 'X')
            MeshRenderer::drawMesh(model, mesh2->mesh(), {0.882, 0.192, 0.161,1});
    }
    MeshRenderer::drawMesh({0,0,0}, {1,1,1}, mesh1->mesh(), {0.165, 0.349, 1.000,1});
    MeshRenderer::drawMesh({1,0,3}, {1,1,1}, mesh2->mesh(), {0.882, 0.192, 0.161,1});
    MeshRenderer::drawMesh({-1,0,3}, {1,1,1}, mesh3->mesh(), {0.906, 0.878, 0.302,1});
    MeshRenderer::end();

    BatchRenderer::begin();
//    BatchRenderer::drawQuad({100,100}, {300,300}, {1,1,0,1}); //2d quad
//    BatchRenderer::drawLine({100,100}, {400,400}, 5.f, {1,0,1,1}); // 2d line
//    BatchRenderer::drawCircle({400, 400}, 5, 10, {1,0,0,1}); // 2d circle
//    BatchRenderer::drawLine({-1,0,3}, {leftSlot,top,0}, .02f, {0.882, 0.192, 0.161,1}); //3d line
    BatchRenderer::drawQuad({0,0,0}, {20, 20}, {0.094, 0.141, 0.176,1}); // 3d quad
    BatchRenderer::end();

}
