#include "testmlp.h"
#include "gtc/random.hpp"

TestMLP::TestMLP()
{
    BatchRenderer::setShader(AssetManager::getShader("batch"));
    MeshRenderer::setShader(AssetManager::getShader("mesh"));

    points = (vec3*)malloc(1000*sizeof(vec3));
    vec2 range = {0.f, glm::pi<float>()};
    graph.setMesh(meshX, meshX, false);

    // points
    for(uint i=0; i<pointCount; i++)
    {
        auto& p = points[i];
        p.x = glm::linearRand(range.x, range.y);
        p.z = glm::linearRand(range.x, range.y);
        p.y = cos(p.x*p.z)*cos(2*p.x);
    }

    graph.setPoints(points, pointCount);

}

void TestMLP::onUpdate(float dt)
{
    GraphicsContext::getCamera()->pointAt({5,5,5});
    GraphicsContext::getCamera()->setFocusPoint({5,5,5});
    accum+=dt/2;

    graph.updateMesh([=](const vec2& p){
        return cos(p.x*p.y+accum)*cos(2*p.x+accum);
    });
    graph.draw({0,0,0});

}

Graph3d::Graph3d(vec2 rangeX, vec2 rangeY, vec2 rangeZ, float scale)
{
    m_rangeX = rangeX;
    m_rangeY = rangeY;
    m_rangeZ = rangeZ;
    m_scale  = scale;
    sphere = MeshRenderer::createCubeSphere(5);
}

void Graph3d::setMesh(uint sizeX, uint sizeZ, bool smooth)
{
    meshX = sizeX;
    meshZ = sizeZ;
    m_smooth = smooth;
    if(meshGrid)
        free(meshGrid);

    meshGrid = (vec3*)malloc(meshX*meshZ*sizeof(vec3));
    float stepX = (float)10/(meshX-1);
    float stepZ = (float)10/(meshZ-1);
    for(uint i=0; i<meshZ; i++)
        for(uint j=0; j<meshX; j++)
        {
            auto& p = meshGrid[i*meshX+j];
            p.x = j*stepX;
            p.z = i*stepZ;
            p.y = 0;
        }
    if(m_smooth)
        mesh = MeshRenderer::createSmoothMeshGrid(meshGrid, meshX, meshZ);
    else
        mesh = MeshRenderer::createQuadMeshGrid(meshGrid, meshX, meshZ);
}

void Graph3d::setPoints(vec3 *_points, uint count)
{
    ASSERT(count);
    nPoints = count;
    if(points)
        free(points);

    points = (vec3*)malloc(nPoints*sizeof(vec3));
    for(uint i=0; i<nPoints; i++)
    {
        auto p = _points[i];
        points[i] = vec3(mapToRange(m_rangeX, {0.f, m_scale}, p.x),
                         mapToRange(m_rangeY, {0.f, m_scale}, p.y),
                         mapToRange(m_rangeZ, {0.f, m_scale}, p.z));
    }
}

void Graph3d::updateMesh(std::function<float(const vec2&)> func)
{
    for(uint i=0; i<meshX*meshX; i++)
    {
        auto& p = meshGrid[i];
        float x1 = mapToRange({0, m_scale}, m_rangeX, p.x);
        float x2 = mapToRange({0, m_scale}, m_rangeZ, p.z);
        p.y = mapToRange({-1,1}, {0, m_scale}, func({x1, x2}));
    }
    if(m_smooth)
        mesh = MeshRenderer::createSmoothMeshGrid(meshGrid, meshX, meshZ);
    else
        mesh = MeshRenderer::createQuadMeshGrid(meshGrid, meshX, meshZ);
}

void Graph3d::draw(const vec3 &pos)
{
    drawGrid(pos);
    MeshRenderer::begin();
    if(meshGrid)
    {
        glDisable(GL_CULL_FACE);
        MeshRenderer::drawMesh(pos, vec3(1.0f), mesh);
        glEnable(GL_CULL_FACE);
    }
    if(points)
        for(size_t i=0; i<nPoints; i++)
            MeshRenderer::drawMesh(points[i], vec3(m_scale*0.01f), sphere, {0.2f,0.2f,0.6f,1});
    MeshRenderer::end();
}

void Graph3d::drawGrid(const vec3& pos)
{
    BatchRenderer::begin();
    float lWidth = m_scale * 0.01f;
    // Draw axes
    BatchRenderer::drawLine(pos, pos+m_scale*vec3(1, 0, 0), lWidth, {0.788, 0.820, 0.851,1});
    BatchRenderer::drawLine(pos, pos+m_scale*vec3(0, 1, 0), lWidth, {0.788, 0.820, 0.851,1});
    BatchRenderer::drawLine(pos, pos+m_scale*vec3(0, 0, 1), lWidth, {0.788, 0.820, 0.851,1});

    // Draw planes
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.2f, 0), pos+m_scale*vec3(1, 0.2f, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.4f, 0), pos+m_scale*vec3(1, 0.4f, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.6f, 0), pos+m_scale*vec3(1, 0.6f, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.8f, 0), pos+m_scale*vec3(1, 0.8f, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.2f), pos+m_scale*vec3(1, 0, 0.2f), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.4f), pos+m_scale*vec3(1, 0, 0.4f), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.6f), pos+m_scale*vec3(1, 0, 0.6f), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.8f), pos+m_scale*vec3(1, 0, 0.8f), lWidth/10, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+m_scale*vec3(0.2f, 0, 0), pos+m_scale*vec3(0.2f, 1, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0.4f, 0, 0), pos+m_scale*vec3(0.4f, 1, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0.6f, 0, 0), pos+m_scale*vec3(0.6f, 1, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0.8f, 0, 0), pos+m_scale*vec3(0.8f, 1, 0), lWidth/10, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.2f), pos+m_scale*vec3(0, 1, 0.2f), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.4f), pos+m_scale*vec3(0, 1, 0.4f), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.6f), pos+m_scale*vec3(0, 1, 0.6f), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0, 0.8f), pos+m_scale*vec3(0, 1, 0.8f), lWidth/10, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.2f, 0), pos+m_scale*vec3(0, 0.2f, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.4f, 0), pos+m_scale*vec3(0, 0.4f, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.6f, 0), pos+m_scale*vec3(0, 0.6f, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0, 0.8f, 0), pos+m_scale*vec3(0, 0.8f, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+m_scale*vec3(0.2f, 0, 0), pos+m_scale*vec3(0.2f, 0, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0.4f, 0, 0), pos+m_scale*vec3(0.4f, 0, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0.6f, 0, 0), pos+m_scale*vec3(0.6f, 0, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+m_scale*vec3(0.8f, 0, 0), pos+m_scale*vec3(0.8f, 0, 1), lWidth/10, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::end();
}
