#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"
#include <functional>

class Graph3d
{
public:
    Graph3d(vec2 rangeX, vec2 rangeY, vec2 rangeZ, float scale);
    void setMesh(uint sizeX, uint sizeZ, bool smooth);
    void setPoints(vec3* points, uint count);
    void updateMesh(std::function<float(const vec2&)> func);
    void draw(const vec3& pos);

private:
    vec3* meshGrid = nullptr;
    uint meshX = 0;
    uint meshZ = 0;
    vec3* points   = nullptr;
    uint nPoints   = 0;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> sphere;
    vec2 m_rangeX;
    vec2 m_rangeY;
    vec2 m_rangeZ;
    float m_scale;
    bool m_smooth;
    void drawGrid(const vec3 &pos);
};

class TestMLP : public Scene
{
public:
    TestMLP();
    ~TestMLP(){/*dontcare*/};
    void onUpdate(float dt);

private:
    vec3* points;
    uint pointCount = 1000;
    uint meshX = 50;
    float accum = 0;
    vec2 rangeXZ = {0.f, glm::pi<float>()};
    Graph3d graph = Graph3d(rangeXZ, {-1, 1}, rangeXZ, 10);
};

