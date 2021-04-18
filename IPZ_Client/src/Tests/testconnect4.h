#pragma once
#include "../Core/application.h"
#include "Connect4/connect4.h"

class TestConnect4
{
public:
    TestConnect4();
    void onUpdate(float dt);

private:
    std::shared_ptr<MeshFile> mesh1, mesh2, mesh3;
    float length = 11.9f;
    float top = 9.1;
    float leftSlot = -5.0f;
    float hOffset = 1.67f;
    float vOffset = 1.4f;
    float hPositions[7];
    float vPositions[6];

    bool animating = false;
    Move currentMove;
    int dropIndex = 0;
    float ySpeed = 0;
    float yPos = 11;

    Connect4* c4;
    alpha_beta_searcher<Move, true>* searcher;

};
