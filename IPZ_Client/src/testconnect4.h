#pragma once
#include "application.h"

class TestConnect4
{
public:
    TestConnect4();
    void onUpdate(float dt);

private:
    std::shared_ptr<MeshFile> mesh1, mesh2, mesh3;
    float length = 11.9f;
    float bottom = 2.10f;
    float leftSlot = -5.0f;
    float hOffset = 1.67f;
    float vOffset = 1.35f;
    float hPositions[7];
    float vPositions[6];
};
