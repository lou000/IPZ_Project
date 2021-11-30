#pragma once
#include "buffer.h"

using namespace glm;

struct MeshVertex{
    vec3 position;
    vec3 normal;
//    vec4 color;
    vec2 texCoords;
};

class Mesh
{
public:
    Mesh(float* vertexData, size_t vCount, uint16* indexData, size_t iCount);
    std::shared_ptr<VertexArray> vao(){return m_vao;}

private:
    std::shared_ptr<VertexArray> m_vao;
};

