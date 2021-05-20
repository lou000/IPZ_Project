#include "testmlp.h"
#include "gtc/random.hpp"

TestMLP::TestMLP()
{
    BatchRenderer::setShader(AssetManager::getShader("batch"));
    MeshRenderer::setShader(AssetManager::getShader("mesh"));
    sphere = MeshRenderer::createCubeSphere(4);
    points = (vec3*)malloc(1000*sizeof(vec3));
    vec2 range = {0.f, glm::pi<float>()};

    // points
    for(int i=0; i<1000; i++)
    {
        auto& p = points[i];
        p.x = glm::linearRand(range.x, range.y);
        p.z = glm::linearRand(range.x, range.y);
        p.y = mapToRange({-1,1}, {0, 10}, cos(p.x*p.z)*cos(2*p.x));

        p.x = mapToRange(range, {0,10}, p.x);
        p.z = mapToRange(range, {0,10}, p.z);
    }

    // mesh grid
    meshGrid = (vec3*)malloc(meshX*meshX*sizeof(vec3));
    colorData = (vec4*)malloc(meshX*meshX*sizeof(vec4));

    float step = (float)glm::pi<float>()/meshX;
    for(uint i=0; i<meshX; i++)
        for(uint j=0; j<meshX; j++)
        {
            auto& p = meshGrid[i*meshX+j];
            p.x = j*step;
            p.z = i*step;
            p.y = mapToRange({-1,1}, {0, 10}, cos(p.x*p.z)*cos(2*p.x));

            int index = (int)mapToRange({0, 10}, {0, 249}, p.y);
            colorData[i*meshX+j] = prettyColors[index];

            p.x = mapToRange(range, {0,10}, p.x);
            p.z = mapToRange(range, {0,10}, p.z);
        }
    originalMesh = MeshRenderer::createMeshGridQuad(meshGrid, meshX, meshX);
}

void TestMLP::onUpdate(float dt)
{
    GraphicsContext::getCamera()->pointAt({5,5,5});
    GraphicsContext::getCamera()->setFocusPoint({5,5,5});


    graph3d({0,0,0}, points, 1000, false);
}

void TestMLP::graph3d(vec3 pos, vec3 *points, size_t count, bool surface, bool smooth)
{

    BatchRenderer::begin();
    // Draw axes
    BatchRenderer::drawLine(pos, pos+vec3(10,  0, 0), 0.1f, {0.788, 0.820, 0.851,1});
    BatchRenderer::drawLine(pos, pos+vec3( 0, 10, 0), 0.1f, {0.788, 0.820, 0.851,1});
    BatchRenderer::drawLine(pos, pos+vec3( 0,  0,10), 0.1f, {0.788, 0.820, 0.851,1});

    // Draw planes
    BatchRenderer::drawLine(pos+vec3(0, 2, 0), pos+vec3(10, 2, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 4, 0), pos+vec3(10, 4, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 6, 0), pos+vec3(10, 6, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 8, 0), pos+vec3(10, 8, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+vec3(0, 0, 2), pos+vec3(10, 0, 2), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 0, 4), pos+vec3(10, 0, 4), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 0, 6), pos+vec3(10, 0, 6), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 0, 8), pos+vec3(10, 0, 8), 0.01f, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+vec3(2, 0, 0), pos+vec3(2, 10, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(4, 0, 0), pos+vec3(4, 10, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(6, 0, 0), pos+vec3(6, 10, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(8, 0, 0), pos+vec3(8, 10, 0), 0.01f, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+vec3(0, 0, 2), pos+vec3(0, 10, 2), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 0, 4), pos+vec3(0, 10, 4), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 0, 6), pos+vec3(0, 10, 6), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 0, 8), pos+vec3(0, 10, 8), 0.01f, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+vec3(0, 2, 0), pos+vec3(0, 2, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 4, 0), pos+vec3(0, 4, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 6, 0), pos+vec3(0, 6, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(0, 8, 0), pos+vec3(0, 8, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::drawLine(pos+vec3(2, 0, 0), pos+vec3(2, 0, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(4, 0, 0), pos+vec3(4, 0, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(6, 0, 0), pos+vec3(6, 0, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});
    BatchRenderer::drawLine(pos+vec3(8, 0, 0), pos+vec3(8, 0, 10), 0.01f, {0.788, 0.820, 0.851, 0.5});

    BatchRenderer::end();




    MeshRenderer::begin();
    // Draw points
    for(size_t i=0; i<count; i++)
    {
        auto p = points[i];
        MeshRenderer::drawMesh(p, vec3(0.1f), sphere, {0.2f,0.2f,0.6f,1});
    }
    glDisable(GL_CULL_FACE);
    MeshRenderer::drawMesh(vec3(0.f), vec3(1.f), originalMesh);
    glEnable(GL_CULL_FACE);

    MeshRenderer::end();
}
