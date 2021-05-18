#include "testmlp.h"

TestMLP::TestMLP()
{
    BatchRenderer::setShader(AssetManager::getShader("batch"));
    MeshRenderer::setShader(AssetManager::getShader("mesh"));

}

void TestMLP::onUpdate(float dt)
{
    GraphicsContext::getCamera()->pointAt({0,0,0});
    GraphicsContext::getCamera()->setFocusPoint({0,0,0});
    graph3d({0,0,0}, nullptr, 0, false);
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

    // Draw points
    if(!surface)
    {

    }

    int vPerEdge = 8;
    float offset = (float)1/(vPerEdge-1);
    int cornerVertices = 8;
    int edgeVertices = (vPerEdge-2) * 12;
    int faceVertices = (vPerEdge-2)*(vPerEdge-2)*6;
    size_t vCount = cornerVertices + edgeVertices + faceVertices;
    vec3* posVerts = (vec3*)alloca(vCount*sizeof(vec3));

    size_t v = 0;
    for(int y = 0; y<vPerEdge; y++)
    {
        // front row
        for (int i = 0; i < vPerEdge; i++)
            posVerts[v++] = vec3(-0.5+i*offset, -0.5+y*offset, 0.5);

        // right row
        for (int i = 1; i < vPerEdge; i++)
            posVerts[v++] = vec3(0.5, -0.5+y*offset, 0.5-i*offset);

        // back row
        for (int i = 1; i < vPerEdge; i++)
            posVerts[v++] = vec3(0.5-i*offset, -0.5+y*offset, -0.5);

        // left row
        for (int i = 1; i < vPerEdge-1; i++)
            posVerts[v++] = vec3(-0.5, -0.5+y*offset, -0.5+i*offset);
    }
    // top face
    int topStart = v;
    for (int z = 1; z < vPerEdge-1; z++)
        for (int x = 1; x < vPerEdge-1; x++)
            posVerts[v++] = vec3(-0.5+x*offset,  0.5, 0.5-z*offset);

    // bottom face
    int bottomStart = v;
    for (int z = 1; z < vPerEdge-1; z++)
        for (int x = 1; x < vPerEdge-1; x++)
            posVerts[v++] = vec3(-0.5+x*offset, -0.5, 0.5-z*offset);

    // INDICES
    int iCount = 6*(vPerEdge-1)*(vPerEdge-1)*2*3; //six faces of (vPerEdge-1)^2 quads 2 tris per quad 3 indices per tri
    uint16* indices = (uint16*)alloca(iCount*sizeof(uint16));
    size_t ind = 0;


    auto indexQuad = [&](uint16 p0, uint16 p1, uint16 p2, uint16 p3){
        indices[ind+0] = p0;
        indices[ind+1] = p1;
        indices[ind+2] = p3;
        indices[ind+3] = p1;
        indices[ind+4] = p3;
        indices[ind+5] = p2;
        ind+=6;
    };

    // sides
    int loop = (vPerEdge-2) * 4 + 4;
    for(int y=0; y<vPerEdge-1; y++)
    {
        int offset = y*loop;
        for(int i=0; i<loop-1; i++)
            indexQuad(offset+i,offset+i+1,offset+i+loop+1, offset+i+loop);
        indexQuad(offset+loop-1, offset+0, offset+loop, offset+loop+loop-1);
    }

    // top
    int bottomLeft  = loop*(vPerEdge-1);
    int bottomRight = loop*(vPerEdge-1)+vPerEdge-1;
    int topRight    = loop*(vPerEdge-1)+(vPerEdge-1)*2;
    int topLeft     = loop*(vPerEdge-1)+(vPerEdge-1)*3;

    int topRowW = vPerEdge-2;
    // top - bottom row
    indexQuad(bottomLeft, bottomLeft+1, topStart, topLeft+vPerEdge-2);
    for(int i=0; i<topRowW-1; i++)
        indexQuad(bottomLeft+i+1, bottomLeft+i+2, topStart+i+1, topStart+i);
    indexQuad(bottomRight-1, bottomRight, bottomRight+1, topStart+topRowW-1);

    // top - middle rows
    for(int i=0; i<topRowW-1; i++)
    {
        // good thing i have to write this only once...
        indexQuad(topLeft+vPerEdge-i-2, topStart+topRowW*i, topStart+topRowW*(i+1), topLeft+topRowW-i-1);
        for(int j=0; j<topRowW-1; j++)
            indexQuad(topStart+topRowW*i+j, topStart+topRowW*i+j+1, topStart+topRowW*(i+1)+j+1, topStart+topRowW*(i+1)+j);
        indexQuad(topStart+topRowW+topRowW*i-1, bottomRight+1+i, bottomRight+1+i+1, topStart+topRowW+topRowW*(i+1)-1);
    }

    // top - top row
    indexQuad(topLeft+1, topStart+topRowW*(topRowW-1), topLeft-1, topLeft);
    for(int i=0; i<topRowW-1; i++)
        indexQuad(topStart+topRowW*(topRowW-1)+i, topStart+topRowW*(topRowW-1)+i+1, topLeft-i-2, topLeft-i-1);
    indexQuad(topStart+topRowW*topRowW-1, topRight-1, topRight, topRight+1);


    // bottom
    auto bottomIndx = ind;
    auto indexQuadInv = [&](uint16 p0, uint16 p1, uint16 p2, uint16 p3){
        indices[ind+0] = p3;
        indices[ind+1] = p2;
        indices[ind+2] = p0;
        indices[ind+3] = p2;
        indices[ind+4] = p0;
        indices[ind+5] = p1;
        ind+=6;
    };
    bottomLeft  = 0;
    bottomRight = (vPerEdge-1);
    topRight    = 2*(vPerEdge-1);
    topLeft     = 3*(vPerEdge-1);


    // bottom - bottom row
    indexQuadInv(bottomLeft, bottomLeft+1, bottomStart, topLeft+topRowW);
    for(int i=0; i<topRowW-1; i++)
        indexQuadInv(bottomLeft+i+1, bottomLeft+i+2, bottomStart+i+1, bottomStart+i);
    indexQuadInv(bottomRight-1, bottomRight, bottomRight+1, bottomStart+topRowW-1);

    // bottom - middle rows
    for(int i=0; i<topRowW-1; i++)
    {
        // good thing i have to write this only once...
        indexQuadInv(topLeft+vPerEdge-i-2, bottomStart+topRowW*i, bottomStart+topRowW*(i+1), topLeft+topRowW-i-1);
        for(int j=0; j<topRowW-1; j++)
            indexQuadInv(bottomStart+topRowW*i+j, bottomStart+topRowW*i+j+1, bottomStart+topRowW*(i+1)+j+1, bottomStart+topRowW*(i+1)+j);
        indexQuadInv(bottomStart+topRowW+topRowW*i-1, bottomRight+1+i, bottomRight+1+i+1, bottomStart+topRowW+topRowW*(i+1)-1);
    }

    // top - top row
    indexQuadInv(topLeft+1, bottomStart+topRowW*(topRowW-1), topLeft-1, topLeft);
    for(int i=0; i<topRowW-1; i++)
        indexQuadInv(bottomStart+topRowW*(topRowW-1)+i, bottomStart+topRowW*(topRowW-1)+i+1, topLeft-i-2, topLeft-i-1);
    indexQuadInv(bottomStart+topRowW*topRowW-1, topRight-1, topRight, topRight+1);


    for(size_t i=0; i<ind; i+=6)
    {
        vec4 col = i>=bottomIndx ? vec4{1,0,0,1} : vec4{0,1,0,1};
        BatchRenderer::drawLine(posVerts[indices[i]],  posVerts[indices[i+1]], 0.01, col);
        BatchRenderer::drawLine(posVerts[indices[i+1]],posVerts[indices[i+2]], 0.01, col);
        BatchRenderer::drawLine(posVerts[indices[i+2]],  posVerts[indices[i+0]], 0.01, col);

        BatchRenderer::drawLine(posVerts[indices[i+3]],  posVerts[indices[i+4]], 0.01, col);
        BatchRenderer::drawLine(posVerts[indices[i+4]],posVerts[indices[i+5]], 0.01, col);
        BatchRenderer::drawLine(posVerts[indices[i+5]],  posVerts[indices[i+3]], 0.01, col);
    }


    BatchRenderer::end();
}
