#include "camera.h"


Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
    :m_fov(fov), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip)
{

}

void Camera::setPosition(vec3 pos)
{

}

void Camera::setRotationX(float degree)
{

}

void Camera::setRotationY(float degree)
{

}

void Camera::setRotation(quat q)
{

}


void Camera::pointAt(vec3 pos)
{

}


void Camera::setFov(float fov)
{

}

void Camera::setAspectRatio(float ratio)
{

}


vec3 Camera::up()
{

}

vec3 Camera::right()
{

}

vec3 Camera::forward()
{

}

void Camera::calculateViewMat()
{

}

void Camera::calculateProjMat()
{

}
