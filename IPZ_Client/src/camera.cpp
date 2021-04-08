#include "camera.h"
#include "application.h"
#include "gtx/quaternion.hpp"

Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
    :m_fov(fov), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip)
{
    updateProjMat();
    updateViewMat();
    onCreate();
}

void Camera::move(vec3 vec)
{
    m_pos += vec;
}

void Camera::pointAt(vec3 pos)
{
    auto lookAt = quatLookAt(normalize(pos - getPos()), {0,1,0});
    auto euler = eulerAngles(lookAt);
    m_rotationX = degrees(euler.x);
    m_rotationY = degrees(euler.y);
    m_rotationZ = degrees(euler.z);
}

quat Camera::getRotation()
{
    return quat({radians(m_rotationX), radians(m_rotationY), radians(m_rotationZ)});
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
    return glm::rotate(getRotation(), vec3(0, 1, 0));
}

vec3 Camera::right()
{
    return glm::rotate(getRotation(), vec3(1, 0, 0));
}

vec3 Camera::forward()
{
    return glm::rotate(getRotation(), vec3(0, 0, -1));
}

void Camera::onUpdate(float dt)
{
    float rotationSpeed = 100.f;
    float xSign = forward().z < 0 ? 1.f : -1.f;
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_W))
        addRotationX(rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_S))
        addRotationX(-rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_A))
        addRotationY(xSign*rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_D))
        addRotationY(-xSign*rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_SPACE))
        pointAt({0,0,0});

    float speed = 3.f;
    vec3 moveVec = {0, 0 ,0};
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_UP))
        moveVec +=  forward() * speed * dt;
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_DOWN))
        moveVec += -forward() * speed * dt;
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_RIGHT))
        moveVec +=  right() * speed * dt;
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_LEFT))
        moveVec += -right() * speed * dt;
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_Q))
        moveVec +=  up() * speed * dt;
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_Z))
        moveVec += -up() * speed * dt;

    move(moveVec);

//    auto angles = eulerAngles(getRotation());
//    LOG("Rotation: x:%f  y:%f  z:%f    Pos: x:%f  y:%f  z:%f",
//        angles.x, angles.y, angles.z, m_pos.x, m_pos.y, m_pos.z);
}

void Camera::onCreate()
{
    setPosition(vec3(0,0,1.2f));
}

void Camera::updateViewMat()
{
    m_viewMat = translate(mat4(1.0f), m_pos)* toMat4(getRotation());
    m_viewMat = inverse(m_viewMat);
}

void Camera::updateProjMat()
{
    m_projMat = perspective(m_fov, m_aspectRatio, m_nearClip, m_farClip);
}
