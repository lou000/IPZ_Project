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

void Camera::addRotationX(float degree)
{
    auto euler = eulerAngles(m_rotation);
    m_rotation = quat({euler.x + radians(degree), euler.y, euler.z});
}

void Camera::addRotationY(float degree)
{
    auto euler = eulerAngles(m_rotation);
    m_rotation = quat({euler.x, euler.y + radians(degree), euler.z});
}

void Camera::addRotationZ(float degree)
{
    auto euler = eulerAngles(m_rotation);
    m_rotation = quat({euler.x, euler.y, euler.z + radians(degree)});
}

void Camera::pointAt(vec3 pos)
{
    m_rotation = quatLookAt(normalize(pos - getPos()), {0,1,0});
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
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_W))
        addRotationX(rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_S))
        addRotationX(-rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_A))
        addRotationY(rotationSpeed*dt);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_D))
        addRotationY(-rotationSpeed*dt);
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
    setPosition(vec3(0,1.2f,0));
    setRotationX(-90);
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
