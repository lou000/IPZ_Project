#pragma once
#include "buffer.h"

using namespace glm;

struct MeshVertex{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};

struct Material
{
    bool textured = false;

    float metallic  = 0.3f;
    float roughness = 0.1f;
    vec4  color     = {0,0,0,1};
};

class Mesh
{
public:
    Mesh(float* vertexData, size_t vCount, uint16* indexData, size_t iCount, Material material = {});
    std::shared_ptr<VertexArray> vao(){return m_vao;}

    Material material;
    std::shared_ptr<VertexArray> m_vao;
};

