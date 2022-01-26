#include "math.h"
#include "../Renderer/camera.h"
#include "../Renderer/light.h"
std::vector<vec4> calcFrustumCornersWS(const mat4& proj, const mat4& cameraView)
{
    const mat4 inv = inverse(proj*cameraView);

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

float distancePointToLine(vec2 v, vec2 w, vec2 p) {
    // Return minimum distance between line segment vw and point p
    const float l2 = pow(distance(v,w), 2);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 == 0.0) return distance(p, v);   // v == w case
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    const float t = glm::max(0.f, glm::min(1.0f, glm::dot(p - v, w - v) / l2));
    const vec2 projection = v + t * (w - v);  // Projection falls on the segment
    return distance(p, projection);
}

mat4 calcDirLightViewProjMatrix(std::shared_ptr<Camera> camera, DirectionalLight dirLight,
                                float nearPlane, float farPlane, float zCorrection)
{
    mat4 proj = perspective(glm::radians(camera->getFov()), camera->getAspectRatio(),
                                         nearPlane, farPlane);
    const auto corners = calcFrustumCornersWS(proj, camera->getViewMatrix());

    vec3 center = vec3(0.f);

    for(auto c : corners)
        center += vec3(c);
    center /= corners.size();

    const mat4 lightView = lookAt(center-dirLight.direction, center, vec3(0,1,0));

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

bool intersectPlane(const vec3 &planeNormal, const vec3 &planePos,
                    const vec3 &rayStartPos, const vec3 &ray, vec3 &intersection)
{
    // assuming vectors are all normalized
    float denom = dot(planeNormal, ray);
    if (denom > 1e-6) {
        vec3 p0l0 = planePos - rayStartPos;
        float t = dot(p0l0, planeNormal) / denom;
        if(t>=0)
        {
            intersection = rayStartPos + t*ray;
            return true;
        }
        else
            return false;
    }
    return false;
}
