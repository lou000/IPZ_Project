#pragma once
#include <gtc/matrix_transform.hpp>
#include <gtx/compatibility.hpp>
#include <gtc/type_ptr.hpp>
#include "gtc/random.hpp"
#include <sstream>
#include <iomanip>
#include <vector>

using namespace glm;

template<typename T>
inline T mapToRange(vec<2,T> startRange, vec<2,T> finalRange, T value)
{
    return (value-startRange.x)/(startRange.y-startRange.x)*(finalRange.y-finalRange.x)+finalRange.x;
}

template<typename T>
inline void arrayShuffle(T* array, size_t count)
{
    for(size_t i=0; i<count-1; i++)
    {
        T temp = array[i];
        size_t rand = glm::linearRand(i, count-1);
        array[i] = array[rand];
        array[rand] = temp;
    }
}

template<typename T>
inline std::string vecToString(vec<4,T> v)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2)
           << v.x << ", " << v.y << ", "
           << v.z << ", " << v.w;
    return stream.str();
}

template<typename T>
inline std::string vecToString(vec<3,T> v)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2)
           << v.x << ", " << v.y << ", "
           << v.z;
    return stream.str();
}

template<typename T>
inline std::string vecToString(vec<2,T> v)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2)
           << v.x << ", " << v.y;
    return stream.str();
}

class Camera;
struct DirectionalLight;
std::vector<vec4> calcFrustumCornersWS(std::shared_ptr<Camera> camera);
mat4 calcDirLightViewProjMatrix(std::shared_ptr<Camera> camera, DirectionalLight dirLight,
                                float nearPlane, float farPlane, float zCorrection);
