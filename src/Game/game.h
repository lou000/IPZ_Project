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
    float health = 100.f;
    bool hurt = false;
    float hurtTime = 1.f;
    float hurtTimer = 0.f;
    vec4 color = {3,3,3,1};
};

struct MobComponent
{
    float speed = 3.2f;
    float health = 100.f;
    float collisionRadius = 3.f;
    float enabledRadius = 20.f;
    float attackCD = 1.0f;

    bool hurt = false;
    float hurtTime = 0.5f;
    float hurtTimer = 0.f;

    float hurtCD = 1.f;
    float hurtCDCurrent = 0.f;
    bool dead = false;

    float attackCDCurrent = 0.f;
};

struct Projectile
{
    Projectile(){};
    Projectile(vec3 dir) : direction(dir){};
    float speed = 6.f;
    float collisionRadius = 4.f;
    vec3 direction = {0,0,0};
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
    float accumTime = 0.f;
    std::shared_ptr<GameCamera> gameCamera;
    vec3 cameraOffset = {0, 20, 15};
    PlayerComponent playerC;
    void updateEntityHeightToTerrain(Entity terrain);
    void generateTrees();
    void generateLanterns();
    void generateRandomStuff();
    void spawnHostiles();
    void fireProjectile(vec3 direction);
    void updatePlayer(float dt, Entity terrain);
    void updateHostiles(float dt, Entity terrain);
    void updateProjectiles(float dt);
    std::vector<Clearing> generateClearings(vec2 terrainSize);
    std::vector<Path> generatePaths(std::vector<Clearing> clearings);

    std::vector<Clearing> m_clearings;
    std::vector<Path> m_paths;
};
