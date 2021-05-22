#pragma once
#include "meshrenderer.h"
#include "batchrenderer.h"
#include <functional>

class Graph3d
{
public:
    Graph3d(vec2 rangeX, vec2 rangeY, vec2 rangeZ, float scale);
    void setMesh(uint sizeX, uint sizeZ, bool smooth);
    void setPoints(vec3* points, uint count);
    void updateMesh(std::function<float(const vec2&)> func);
    void animateTo(std::function<float (const vec2 &)> func, float time);
    void draw(const vec3& pos, float dt);

    bool animating(){return m_animating;}

private:
    bool m_animating  = false;
    float maxAnimTime = 0;
    float animTime = 0;
    float* animValsStart = nullptr;
    float* animValsStop  = nullptr;
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
