#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"

class TestMLP : public Scene
{
public:
    TestMLP();
    ~TestMLP(){/*dontcare*/};
    void onUpdate(float dt);
    void graph3d(vec3 pos, vec3* points, size_t count, bool surface, bool smooth = false);

private:
    std::shared_ptr<Mesh> sphere;
    vec3* points;
    vec3* meshGrid;
    vec4* colorData;
    uint meshX = 50;
    float accum = 0;
    std::shared_ptr<Mesh> originalMesh;
};

