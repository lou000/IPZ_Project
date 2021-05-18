#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"

class TestMLP : public Scene
{
public:
    TestMLP();
    ~TestMLP(){/*dontcare*/};
    void onUpdate(float dt);
    static void graph3d(vec3 pos, vec3* points, size_t count, bool surface, bool smooth = false);
};

