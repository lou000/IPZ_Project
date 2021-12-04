#pragma once
#include "buffer.h"

using namespace glm;

struct MeshVertex{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};

struct MeshVertexColored{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec4 color;
};

class Mesh
{
public:
    Mesh(float* vertexData, size_t vCount, uint16* indexData, size_t iCount, bool textured = false);
    std::shared_ptr<VertexArray> vao(){return m_vao;}

    bool textured = false;

    // pbr properties if not present in textures
    float metallic = 0.3;
    float roughness = 0.1;

    std::shared_ptr<VertexArray> m_vao;
};

