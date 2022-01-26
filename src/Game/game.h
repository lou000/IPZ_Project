#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"

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
};
