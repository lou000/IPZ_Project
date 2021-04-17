#pragma once
#include "application.h"


class Test1
{
public:
    Test1();
    void onUpdate(float dt);

private:
    std::vector<vec2> randomPos;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> texture2;
};

