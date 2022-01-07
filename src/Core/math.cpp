#include "math.h"
#include "../Renderer/camera.h"
#include "../Renderer/light.h"
std::vector<vec4> calcFrustumCornersWS(std::shared_ptr<Camera> camera)
{
    const mat4 inv = inverse(camera->getViewProjectionMatrix());

    std::vector<vec4> corners;

    for (uint x=0; x<2; ++x)
        for (uint y=0; y<2; ++y)
            for (uint z=0; z<2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4( 2.0f * x - 1.0f,
                                                     2.0f * y - 1.0f,
                                                     2.0f * z - 1.0f, 1.0f);
                corners.push_back(pt / pt.w);
            }

    return corners;
}

mat4 calcDirLightViewProjMatrix(std::shared_ptr<Camera> camera, std::shared_ptr<DirectionalLight> dirLight,
                                float nearPlane, float farPlane, float zCorrection)
{
    mat4 proj = perspective(glm::radians(camera->getFov()), camera->getAspectRatio(),
                                         nearPlane, farPlane);
    const auto corners = calcFrustumCornersWS(camera);

    vec3 center = vec3(0.f);

    for(auto c : corners)
        center += vec3(c);

    center = center/4.f;
    const mat4 lightView = lookAt(center+dirLight->direction, center, vec3(0,1,0));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::min();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = min(minX, trf.x);
        maxX = max(maxX, trf.x);
        minY = min(minY, trf.y);
        maxY = max(maxY, trf.y);
        minZ = min(minZ, trf.z);
        maxZ = max(maxZ, trf.z);
    }

    if (minZ < 0)
        minZ *= zCorrection;
    else
        minZ /= zCorrection;

    if (maxZ < 0)
        maxZ /= zCorrection;
    else
        maxZ *= zCorrection;

    const mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    return lightProjection * lightView;
}
