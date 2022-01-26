#pragma once
#include "buffer.h"
#include "camera.h"

using namespace glm;

struct MeshVertex{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};


//FIXME: This should be in "math" file
struct AABB
{
    AABB() : AABB({-std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity()},
                 {std::numeric_limits<float>::infinity(),
                  std::numeric_limits<float>::infinity(),
                  std::numeric_limits<float>::infinity()}){}

    AABB(const glm::vec3& min, const glm::vec3& max)
    : center{ (max + min) * 0.5f }, extents{ max.x - center.x, max.y - center.y, max.z - center.z }
    {}

    AABB(const glm::vec3& inCenter, float iI, float iJ, float iK)
    : center{ inCenter }, extents{ iI, iJ, iK }
    {}

    glm::vec3 center{ 0.f, 0.f, 0.f };
    glm::vec3 extents{ 0.f, 0.f, 0.f };

    bool isOnOrForwardPlan(const Plan& plan) const
    {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = extents.x * std::abs(plan.normal.x) + extents.y * std::abs(plan.normal.y) +
                        extents.z * std::abs(plan.normal.z);

        return -r <= plan.getSignedDistanceToPlan(center);
    }

    bool isOnFrustum(const Frustum& camFrustum, const mat4& transform) const
    {
        //Get global scale thanks to our transform
        const glm::vec3 globalCenter{ transform * glm::vec4(center, 1.f) };

        // Scaled orientation
        const glm::vec3 right = transform[0] * extents.x;
        const glm::vec3 up = transform[1] * extents.y;
        const glm::vec3 forward = -transform[2] * extents.z;

        const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
                            std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
                            std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

        const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
                            std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
                            std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

        const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
                            std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
                            std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

        const AABB globalAABB(globalCenter, newIi, newIj, newIk);

        return (globalAABB.isOnOrForwardPlan(camFrustum.leftFace) &&
                globalAABB.isOnOrForwardPlan(camFrustum.rightFace) &&
                globalAABB.isOnOrForwardPlan(camFrustum.topFace) &&
                globalAABB.isOnOrForwardPlan(camFrustum.bottomFace) &&
                globalAABB.isOnOrForwardPlan(camFrustum.nearFace) &&
                globalAABB.isOnOrForwardPlan(camFrustum.farFace));
    };
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

