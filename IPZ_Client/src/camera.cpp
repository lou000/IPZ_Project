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
    //Borrowed from https://stackoverflow.com/questions/18172388/glm-quaternion-lookat-function
    glm::vec3 direction = pos-getPos();
    float directionLength = length(direction);
    quat rotation = quat(1, 0, 0, 0);

    // Check if the direction is valid; Also deals with NaN
    if(!(directionLength > 0.0001))
    {
        auto euler = eulerAngles(rotation);
        m_rotationX = euler.x;
        m_rotationY = euler.y;
//        m_rotationZ = euler.z;
        return;
    }

    // Normalize direction
    direction /= directionLength;

    // Is the normal up (nearly) parallel to direction?
    if(glm::abs(glm::dot(direction, up())) > .9999f) {
        rotation = glm::quatLookAt(direction, up());
        auto euler = eulerAngles(rotation);
        m_rotationX = euler.x;
        m_rotationY = euler.y;
        m_rotationZ = euler.z;
    }
    else {
        rotation = glm::quatLookAt(direction, up());
        auto euler = eulerAngles(rotation);
        m_rotationX = euler.x;
        m_rotationY = euler.y;
        m_rotationZ = euler.z;
    }
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

void Camera::onUpdate()
{
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_W))
        addRotationX(1.5f);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_S))
        addRotationX(-1.5f);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_A))
        addRotationY(1.5f);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_D))
        addRotationY(-1.5f);
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_SPACE))
        pointAt({0,0,0});

    float speed = 0.2f;
    vec3 moveVec = {0, 0 ,0};
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_UP))
        moveVec += speed * forward();
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_DOWN))
        moveVec += -speed * forward();
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_RIGHT))
        moveVec +=  speed * right();
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_LEFT))
        moveVec += -speed * right();
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_Q))
        moveVec +=  speed * up();
    if(glfwGetKey(App::getWindowHandle(), GLFW_KEY_Z))
        moveVec += -speed * up();

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
