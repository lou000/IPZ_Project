#include "meshrenderer.h"
#include "shader.h"

std::shared_ptr<Mesh> MeshRenderer::createCubeSphere(int vPerEdge)
{
    ASSERT(vPerEdge>=2);

    float offset = (float)1/(vPerEdge-1);
    int cornerVertices = 8;
    int edgeVertices = (vPerEdge-2) * 12;
    int faceVertices = (vPerEdge-2)*(vPerEdge-2)*6;
    uint vCount = cornerVertices + edgeVertices + faceVertices;
    MeshVertex* vertices = (MeshVertex*)malloc(vCount*sizeof(MeshVertex));

    size_t v = 0;
    for(int y = 0; y<vPerEdge; y++)
    {
        // front row
        for (int i = 0; i < vPerEdge; i++)
            vertices[v++].position = vec4(-0.5+i*offset, -0.5+y*offset, 0.5, 0);

        // right row
        for (int i = 1; i < vPerEdge; i++)
            vertices[v++].position = vec4(0.5, -0.5+y*offset, 0.5-i*offset, 0);

        // back row
        for (int i = 1; i < vPerEdge; i++)
            vertices[v++].position = vec4(0.5-i*offset, -0.5+y*offset, -0.5, 0);

        // left row
        for (int i = 1; i < vPerEdge-1; i++)
            vertices[v++].position = vec4(-0.5, -0.5+y*offset, -0.5+i*offset, 0);
    }
    // top face
    int topStart = (int)v;
    for (int z = 1; z < vPerEdge-1; z++)
        for (int x = 1; x < vPerEdge-1; x++)
            vertices[v++].position = vec4(-0.5+x*offset,  0.5, 0.5-z*offset, 0);

    // bottom face
    int bottomStart = (int)v;
    for (int z = 1; z < vPerEdge-1; z++)
        for (int x = 1; x < vPerEdge-1; x++)
            vertices[v++].position = vec4(-0.5+x*offset, -0.5, 0.5-z*offset, 0);

    // INDICES
    int iCount = 6*(vPerEdge-1)*(vPerEdge-1)*2*3; //six faces of (vPerEdge-1)^2 quads 2 tris per quad 3 indices per tri
    uint32* indices = (uint32*)malloc(iCount*sizeof(uint32));
    size_t ind = 0;


    auto indexQuad = [&](uint32 p0, uint32 p1, uint32 p2, uint32 p3){
        indices[ind+0] = p0;
        indices[ind+1] = p1;
        indices[ind+2] = p3;

        indices[ind+3] = p3;
        indices[ind+4] = p1;
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
    auto indexQuadInv = [&](uint32 p0, uint32 p1, uint32 p2, uint32 p3){
        indices[ind+0] = p3;
        indices[ind+1] = p2;
        indices[ind+2] = p0;

        indices[ind+3] = p0;
        indices[ind+4] = p2;
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

    // do the spherification and normals
    for(uint i=0; i<vCount; i++)
    {
        auto normal = normalize(vertices[i].position);
        vertices[i].position = normal;
        vertices[i].normal   = normal;
        //tex coords maybe here
    }

    auto m = std::make_shared<Mesh>((float*)vertices, vCount, indices, iCount);
    free(vertices);
    free(indices);
    return m;
}

std::shared_ptr<Mesh> MeshRenderer::createQuadMeshGrid(vec4 *points, uint xSize, uint zSize)
{
    uint vCount = (xSize-1)*(zSize-1)*4; // num of quads
    uint iCount = (xSize-1)*(zSize-1)*2*3;

    MeshVertex* vertices = (MeshVertex*)calloc(vCount, sizeof(MeshVertex));
    uint32* indices      = (uint32*)malloc(iCount * sizeof(uint32));

    size_t ind = 0;
    auto indexQuad = [&](uint32 p0, uint32 p1, uint32 p2, uint32 p3){
        indices[ind+0] = p0;
        indices[ind+1] = p1;
        indices[ind+2] = p3;

        indices[ind+3] = p3;
        indices[ind+4] = p1;
        indices[ind+5] = p2;

        ind+=6;
    };

    for(uint z=0; z<zSize-1; z++)
    {
        for(uint x=0; x<xSize-1; x++)
        {
            uint i  = z*xSize+x;
            uint i0 = i+xSize;
            uint i1 = i+xSize+1;
            uint i2 = i+1;
            uint i3 = i;

            uint j  = 4*z*(xSize-1)+2*x;
            uint j0 = j+2*(xSize-1);
            uint j1 = j+2*(xSize-1)+1;
            uint j2 = j+1;
            uint j3 = j;


            indexQuad(j0, j1, j2, j3);
            vec4 normal1 = vec4(cross(vec3(points[i0]-points[i1]), vec3(points[i0]-points[i3])), 0);
            vec4 normal2 = vec4(cross(vec3(points[i2]-points[i3]), vec3(points[i2]-points[i1])), 0);

            vertices[j0].position = points[i0];
            vertices[j1].position = points[i1];
            vertices[j2].position = points[i2];
            vertices[j3].position = points[i3];

            vertices[j0].normal = normalize(normal1);
            vertices[j1].normal = normalize(normal1+normal2);
            vertices[j2].normal = normalize(normal2);
            vertices[j3].normal = normalize(normal1+normal2);
        }
    }

    auto m = std::make_shared<Mesh>((float*)vertices, vCount, indices, iCount);
    free(indices);
    free(vertices);
    return m;
}

std::shared_ptr<Model> MeshRenderer::createTriMeshGrid(const std::string& name, uint xSize, uint zSize)
{
    uint vCount = xSize*zSize*6; // num of triangle verts is the same as indices
    uint iCount = xSize*zSize*6;

    MeshVertex* vertices = (MeshVertex*)calloc(vCount, sizeof(MeshVertex));
    uint32* indices      = (uint32*)malloc(iCount * sizeof(uint32));

    size_t ind = 0;

    for(uint z=0; z<zSize; z++)
    {
        for(uint x=0; x<xSize; x++)
        {
            vec4 pos0 = {x,   0, z  , 0};
            vec4 pos1 = {x+1, 0, z+1, 0};
            vec4 pos2 = {x,   0, z+1, 0};
            vec4 pos3 = {x,   0, z  , 0};
            vec4 pos4 = {x+1, 0, z  , 0};
            vec4 pos5 = {x+1, 0, z+1, 0};

            vertices[ind+2].position = pos0;
            vertices[ind+1].position = pos1;
            vertices[ind+0].position = pos2;
            vertices[ind+5].position = pos3;
            vertices[ind+4].position = pos4;
            vertices[ind+3].position = pos5;

            vec4 up = {0,1,0,0};
            vertices[ind+0].normal = normalize(up);
            vertices[ind+1].normal = normalize(up);
            vertices[ind+2].normal = normalize(up);
            vertices[ind+3].normal = normalize(up);
            vertices[ind+4].normal = normalize(up);
            vertices[ind+5].normal = normalize(up);

            indices[ind+0] = ind+0;
            indices[ind+1] = ind+1;
            indices[ind+2] = ind+2;
            indices[ind+3] = ind+3;
            indices[ind+4] = ind+4;
            indices[ind+5] = ind+5;

            ind+=6;
        }
    }

    Material mat;
    mat.metallic = 0;
    mat.roughness = 0.8;
    mat.color = {1,1,1,1};

    auto m = std::make_shared<Mesh>((float*)vertices, vCount, indices, iCount, mat);
    free(indices);
    free(vertices);
    return std::make_shared<Model>(name, std::vector<std::shared_ptr<Mesh>>{m});
}

std::shared_ptr<Mesh> MeshRenderer::createSmoothMeshGrid(vec4 *points, uint xSize, uint zSize) //left to right, top to bottom
{
    uint vCount = xSize*zSize;
    uint iCount = (xSize-1)*(zSize-1)*2*3;
    MeshVertex* vertices = (MeshVertex*)calloc(vCount, sizeof(MeshVertex));
    uint32* indices      = (uint32*)malloc(iCount*sizeof(uint32));

    size_t ind = 0;
    auto indexQuad = [&](uint32 p0, uint32 p1, uint32 p2, uint32 p3){
        indices[ind+0] = p0;
        indices[ind+1] = p1;
        indices[ind+2] = p3;

        indices[ind+3] = p3;
        indices[ind+4] = p1;
        indices[ind+5] = p2;

        ind+=6;
    };

    for(uint z=0; z<zSize-1; z++)
    {
        for(uint x=0; x<xSize-1; x++)
        {
            uint i = z*xSize+x;
            uint i0 = i+xSize;
            uint i1 = i+xSize+1;
            uint i2 = i+1;
            uint i3 = i;

            indexQuad(i0, i1, i2, i3);
            vec4 normal1 = vec4(cross(vec3(points[i0]-points[i1]), vec3(points[i0]-points[i3])), 0);
            vec4 normal2 = vec4(cross(vec3(points[i2]-points[i3]), vec3(points[i2]-points[i1])), 0);

            vertices[i0].position = points[i0];
            vertices[i1].position = points[i1];
            vertices[i2].position = points[i2];
            vertices[i3].position = points[i3];

            vertices[i0].normal += normal1;
            vertices[i1].normal += normal1+normal2;
            vertices[i2].normal += normal2;
            vertices[i3].normal += normal1+normal2;
        }
    }
    for(uint i=0; i<xSize*zSize; i++)
        vertices[i].normal = normalize(vertices[i].normal);

    auto m = std::make_shared<Mesh>((float*)vertices, vCount, indices, iCount);
    free(indices);
    free(vertices);
    return m;
}
