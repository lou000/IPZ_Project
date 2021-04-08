#include "camera.h"
#include "gtx/quaternion.hpp"

Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
    :m_fov(fov), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip)
{
    updateProjMat();
    updateViewMat();
}

void Camera::setRotationX(float degree)
{
    auto euler = eulerAngles(m_rotation);
    m_rotation = quat({radians(degree), euler.y, euler.z});
}

void Camera::setRotationY(float degree)
{
    auto euler = eulerAngles(m_rotation);
    m_rotation = quat({euler.x, radians(degree), euler.z});
}

void Camera::setRotationZ(float degree)
{
    auto euler = eulerAngles(m_rotation);
    m_rotation = quat({euler.x, euler.y, radians(degree)});
}

void Camera::pointAt(vec3 pos)
{
    m_rotation = quatLookAt(normalize(pos), {0,1,0});
}

float Camera::getRotationX()
{
    auto euler = eulerAngles(m_rotation);
    return degrees(euler.x);
}

float Camera::getRotationY()
{
    auto euler = eulerAngles(m_rotation);
    return degrees(euler.y);
}

float Camera::getRotationZ()
{
    auto euler = eulerAngles(m_rotation);
    return degrees(euler.z);
}

mat4 Camera::getViewMatrix()
{
    updateViewMat();
    return m_viewMat;
}

mat4 Camera::getProjMatrix()
{
    updateProjMat();
    return m_projMat;
}

mat4 Camera::getViewProjectionMatrix()
{
    updateProjMat();
    updateViewMat();
    return m_projMat*m_viewMat;
}


vec3 Camera::up()
{
    return m_rotation * vec3(0, 1, 0);
}

vec3 Camera::right()
{
    return m_rotation * vec3(1, 0, 0);
}

vec3 Camera::forward()
{
    return m_rotation * vec3(0, 0, -1);
}

void Camera::updateViewMat()
{
    m_viewMat = translate(mat4(1.0f), m_pos)* toMat4(m_rotation);
    m_viewMat = inverse(m_viewMat);
}

void Camera::updateProjMat()
{
    m_projMat = perspective(m_fov, m_aspectRatio, m_nearClip, m_farClip);
}
