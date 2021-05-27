#pragma once
#include "meshrenderer.h"
#include "batchrenderer.h"
#define DEFAULT_2D_SIZE 1000
#include <functional>

class Graph3d
{
    struct Point{
        vec3 pos;
        vec4 color;
    };

public:
    Graph3d(vec2 rangeX, vec2 rangeY, vec2 rangeZ, float scale);
    void setMesh(uint sizeX, uint sizeZ, bool smooth);
    void addPoints(vec3* points, uint count, const vec4& color = {1,1,1,1});
    void updateMesh(std::function<float(const vec2&)> func);
    void animateTo(std::function<float (const vec2 &)> func, float time);
    void setPalette(vec4* palette, uint count);
    void setPaletteBlend(std::vector<vec4> blendColors, uint steps);
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
    Point* points   = nullptr;
    uint nPoints   = 0;
    vec4* palette  = nullptr;
    uint paletteCount;

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> sphere;
    vec2 m_rangeX;
    vec2 m_rangeY;
    vec2 m_rangeZ;
    float m_scale;
    bool m_smooth;

    void drawGrid(const vec3 &pos);
};

class Graph2d
{
    struct Point{
        vec2 pos;
        vec4 color;
    };
    struct Line{
        vec2 start;
        vec2 end;
        vec4 color;
    };

public:
    Graph2d();
    void setRange(vec2 rangeX, vec2 rangeY);
    void setMesh(uint sizeX, uint sizeY, const vec4& borderColor = {1,1,1,1});
    void addPoints(vec2* points, uint count, const vec4& color = {1,1,1,1});
    void clearPoints();
    void addLine(const vec2& start, const vec2& end, const vec4& color = {1,1,1,1});
    void clearLines();
    void updateMesh(std::function<float(const vec2&)> func, std::function<bool(float)> eval);
    void draw(const vec2& pos, const vec2& size);

private:
    std::vector<Point> points;
    std::vector<Line> lines;

    vec2 m_rangeX = {0, DEFAULT_2D_SIZE};
    vec2 m_rangeY = {0, DEFAULT_2D_SIZE};

};
