#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"


class Test1 : public Scene
{
public:
    Test1();
    ~Test1(){/*dontcare*/};
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;

private:
    std::vector<vec2> randomPos;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> texture2;
};

