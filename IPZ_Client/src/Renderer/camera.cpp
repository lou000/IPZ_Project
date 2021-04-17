#include "camera.h"
#include "../Core/application.h"
#include "gtx/quaternion.hpp"
#include "gtx/transform.hpp"

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

float Camera::getRotationX()
{
    return eulerAngles(m_rotation).x;
}

float Camera::getRotationY()
{
    return eulerAngles(m_rotation).y;
}

float Camera::getRotationZ()
{
    return eulerAngles(m_rotation).z;
}

vec3 Camera::getMouseRay()
{
    auto mousePos = App::getMousePos();
    auto wSize = App::getWindowSize();
    vec4 rayEye = inverse(getProjMatrix())*vec4{(2.0f*mousePos.x)/wSize.x-1.0f, 1.0f-(2.0f*mousePos.y)/wSize.y, -1, 1};
    rayEye = vec4(rayEye.x, rayEye.y, -1, 0);
    vec4 rayWorld = inverse(getViewMatrix())*rayEye;

    return normalize(vec3(rayWorld.x, rayWorld.y, rayWorld.z));

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
    //MOUSE
    float offset = (float)App::getMouseScrollChange();
    if(offset!=0)
    {
        float speed = 0.5f;
        m_pos += offset * speed * forward();
    }

    if(App::getMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT))
    {
        auto mChange = App::getMousePosChange();
        if(!firstMouseClick)
        {
            float sens = 0.0015f;
            auto rot = angleAxis(-mChange.y*sens, right());
            auto rot2 = angleAxis(-mChange.x*sens, up());
            m_pos = m_focusPoint + (rot * (m_pos-m_focusPoint));
            m_pos = m_focusPoint + (rot2 * (m_pos-m_focusPoint));
            pointAt(m_focusPoint);
        }
        else
        {
            App::disableCursor(true);
            firstMouseClick = false;
        }
    }
    else
    {
        firstMouseClick = true;
        App::disableCursor(false);
    }

    //KEYBOARD //getKey is just wrong here, rename it to getKeyOnce and setup a function for glfwGetKey
    float rotationSpeed = 30.f*dt;
    if(App::getKey(GLFW_KEY_W))
        addRotationX(rotationSpeed);
    if(App::getKey(GLFW_KEY_S))
        addRotationX(-rotationSpeed);
    if(App::getKey(GLFW_KEY_A))
        addRotationY(rotationSpeed);
    if(App::getKey(GLFW_KEY_D))
        addRotationY(-rotationSpeed);
//    if(App::getKey(GLFW_KEY_SPACE))
//        pointAt({0,0,0});

    float speed = 3.f*dt;
    vec3 moveVec = {0, 0 ,0};
    if(App::getKey(GLFW_KEY_UP))
    {
        moveVec +=  forward();
        moveVec.y = 0;
        moveVec = normalize(moveVec)* speed;
    }
    if(App::getKey(GLFW_KEY_DOWN))
    {
        moveVec += -forward();
        moveVec.y = 0;
        moveVec = normalize(moveVec)* speed;
    }
    if(App::getKey(GLFW_KEY_RIGHT))
        moveVec +=  right() * speed;
    if(App::getKey(GLFW_KEY_LEFT))
        moveVec += -right() * speed;


    if(App::getKey(GLFW_KEY_Q))
        moveVec +=  vec3(0,1,0) * speed;
    if(App::getKey(GLFW_KEY_Z))
        moveVec += vec3(0, -1, 0) * speed;

    move(moveVec);
}

void Camera::onCreate()
{
    setPosition(vec3(0,1.2f,0.01f));
    pointAt({0,0,0});
}

void Camera::updateViewMat()
{
    m_viewMat = translate(mat4(1.0f), m_pos)* toMat4(getRotation());
    m_viewMat = inverse(m_viewMat);
}

void Camera::updateProjMat()
{
    m_projMat = perspective(radians(m_fov), m_aspectRatio, m_nearClip, m_farClip);
}
