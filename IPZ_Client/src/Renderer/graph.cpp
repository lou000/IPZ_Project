#include "graph.h"

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
    {
        free(meshGrid);
        free(animValsStop);
        free(animValsStart);
    }
    meshGrid = (vec3*)malloc(meshX*meshZ*sizeof(vec3));
    animValsStop  = (float*)malloc(meshX*meshZ*sizeof(float));
    animValsStart = (float*)malloc(meshX*meshZ*sizeof(float));


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
        float y = clamp(func({x1, x2}), -1.f, 1.f);
        p.y = mapToRange({-1,1}, {0, m_scale}, y);
    }
    if(m_smooth)
        mesh = MeshRenderer::createSmoothMeshGrid(meshGrid, meshX, meshZ);
    else
        mesh = MeshRenderer::createQuadMeshGrid(meshGrid, meshX, meshZ);
}

void Graph3d::animateTo(std::function<float(const vec2&)> func, float time)
{
    for(uint i=0; i<meshX*meshX; i++)
    {
        auto& p = meshGrid[i];
        float x1 = mapToRange({0, m_scale}, m_rangeX, p.x);
        float x2 = mapToRange({0, m_scale}, m_rangeZ, p.z);
        float y = clamp(func({x1, x2}), -1.f, 1.f);
        animValsStart[i] = meshGrid[i].y;
        animValsStop[i]  = mapToRange({-1,1}, {0, m_scale}, y);
    }
    animTime = 0;
    maxAnimTime = time;
    m_animating = true;
}

void Graph3d::draw(const vec3 &pos, float dt)
{
    if(m_animating)
    {
        for(uint i=0; i<meshX*meshX; i++)
            meshGrid[i].y = glm::lerp(animValsStart[i], animValsStop[i], animTime/maxAnimTime);

        if(m_smooth)
            mesh = MeshRenderer::createSmoothMeshGrid(meshGrid, meshX, meshZ);
        else
            mesh = MeshRenderer::createQuadMeshGrid(meshGrid, meshX, meshZ);

        animTime+=dt;
        if(animTime>maxAnimTime)
        {
            animTime = 0;
            m_animating = false;
        }
    }
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
            MeshRenderer::drawMesh(pos+points[i], vec3(m_scale*0.01f), sphere, {0.2f,0.2f,0.6f,1});
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
