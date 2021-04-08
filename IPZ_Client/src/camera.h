#pragma once
#include "utilities.h"

using namespace glm;
class Camera{

public:
    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    // Speed: in case of multiple transformations to the camera, the m_rotation quaternion
    // and view matrix will be updated multiple times, when using these functions
    // we should update the matrices only when getViewProjectionMatrix is called
    // and update m_rotation only when getRotation is called
    // this approach may be error prone...

    void setPosition(vec3 pos)      {m_pos = pos;}
    void setRotation(quat q)        {m_rotation = q;}
    void setFov(float fov)          {m_fov = fov;}
    void setAspectRatio(float ratio){m_aspectRatio = ratio;}
    void setRotationX(float degree);
    void setRotationY(float degree);
    void setRotationZ(float degree);
    void pointAt(vec3 pos);

    vec3 getPos()         {return m_pos;}
    float getFov()        {return m_fov;}
    float getAspectRatio(){return m_aspectRatio;}
    quat getRotation(){return m_rotation;}
    float getRotationX();
    float getRotationY();
    float getRotationZ();

    mat4 getViewMatrix();
    mat4 getProjMatrix();
    mat4 getViewProjectionMatrix();


    vec3 up();
    vec3 right();
    vec3 forward();


private:
    vec3 m_pos = {0, 0, 0};
    float m_fov  = 90;
    float m_aspectRatio = 1;
    float m_nearClip    = 0.1f;
    float m_farClip     = 1000;

    quat m_rotation={};
    mat4 m_viewMat;
    mat4 m_projMat;

    void updateViewMat();
    void updateProjMat();
};
