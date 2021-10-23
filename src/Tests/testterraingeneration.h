#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"

class TestTerrainGeneration : public Scene
{
public:
    TestTerrainGeneration();
    ~TestTerrainGeneration(){/*dontcare*/};

    virtual void onUpdate(float dt) override;
    virtual void onStart() override;

private:


};

