#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"
#include "gamecamera.h"

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

struct PlayerComponent
{
    float speed = 6.f;
    vec3 moveTarget = {0,0,0};
    bool moving = false;
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

    std::shared_ptr<GameCamera> gameCamera;
    vec3 cameraOffset = {0, 20, 15};
    PlayerComponent playerC;
    void updateEntityHeightToTerrain(Entity terrain);
    void generateTrees();
    void generateLanterns();
    void updatePlayer(float dt, Entity terrain);
    std::vector<Clearing> generateClearings(vec2 terrainSize);
    std::vector<Path> generatePaths(std::vector<Clearing> clearings);

    std::vector<Clearing> m_clearings;
    std::vector<Path> m_paths;
};
