#pragma once
#include "Connect4/connect4.h"
#include "../Core/application.h"
#include "../Core/scene.h"


class TestConnect4 : public Scene
{
public:
    TestConnect4();
    ~TestConnect4(){/*dontcare*/}
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;
    virtual void onDebugDraw() override;
    virtual void onGuiRender() override;

private:
    uint entityCount = 4;
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
