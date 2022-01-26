#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"

struct Clearing
{
    vec2 pos = {0,0};
    float radius = 0;
};

struct Path
{
    vec2 pointBegin, pointEnd;
    float width;
};

class Game : public Scene
{
public:
    Game();
    ~Game() = default;

    virtual void onStart() override;
    virtual void onUpdate(float dt) override;
    virtual void onDebugDraw() override;
    virtual void onGuiRender() override;

private:

    void updateEntityHeightToTerrain(Entity terrain);
    std::vector<Clearing> generateClearings(vec2 terrainSize);
    std::vector<Path> generatePaths(std::vector<Clearing> clearings);
};
