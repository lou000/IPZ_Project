#pragma once
#include "utilities.h"

using namespace glm;
class Camera{

public:
    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    // Most of this functionality should be moved to entity base class class

    void setPosition(vec3 pos)      {m_pos = pos;}
    void move(vec3 vec);
    void setFov(float fov)          {m_fov = fov;}
    void setAspectRatio(float ratio){m_aspectRatio = ratio;}
    void setRotationX(float degree){m_rotationX = degree;}
    void setRotationY(float degree){m_rotationY = degree;}
    void setRotationZ(float degree){m_rotationZ = degree;}
    void addRotationX(float degree){m_rotationX += degree;}
    void addRotationY(float degree){m_rotationY += degree;}
    void addRotationZ(float degree){m_rotationZ += degree;}
    void pointAt(vec3 pos);

    vec3 getPos()          {return m_pos;}
    float getFov()         {return m_fov;}
    float getAspectRatio(){return m_aspectRatio;}
    float getRotationX()  {return m_rotationX;}
    float getRotationY()  {return m_rotationY;}
    float getRotationZ()  {return m_rotationZ;}
    quat getRotation();

    mat4 getViewMatrix();
    mat4 getProjMatrix();
    mat4 getViewProjectionMatrix();

    vec3 up();
    vec3 right();
    vec3 forward();

    void onUpdate(float dt); // should be in entity
    void onCreate(); // should be in entity


private:
    vec3 m_pos = {0, 0, 0};// should be in entity
    float m_fov  = 90;
    float m_aspectRatio = 1;
    float m_nearClip    = 0.1f;
    float m_farClip     = 1000;

    float m_rotationX = 0;
    float m_rotationY = 0;
    float m_rotationZ = 0;
    mat4 m_viewMat;
    mat4 m_projMat;

    void updateViewMat();
    void updateProjMat();
};
