#pragma once
#include "buffer.h"

using namespace glm;

struct MeshVertex{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};


//FIXME: This should be in "math" file
struct AABB
{
    vec3 min = {std::numeric_limits<float>::infinity(),
                std::numeric_limits<float>::infinity(),
                std::numeric_limits<float>::infinity()};
    vec3 max = {-std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity(),
                -std::numeric_limits<float>::infinity()};
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
    Mesh(float* vertexData, size_t vCount, uint32* indexData, size_t iCount, Material material = {}, AABB boundingBox = {});
    std::shared_ptr<VertexArray> vao(){return m_vao;}
    Material material;
    AABB boundingBox;
    std::shared_ptr<VertexArray> m_vao;
};

