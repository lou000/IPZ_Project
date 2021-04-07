#pragma once
#include "utilities.h"

using namespace glm;
class Camera{

public:
    Camera(float fove, float apectRatio, float nearClip, float farClip);

    void setPosition(vec3 pos);
    void setRotationX(float degree);
    void setRotationY(float degree);
    void setRotationZ(float degree);
    void setRotation(quat q);
    void pointAt(vec3 pos);
    void setFov(float fov);
    void setAspectRatio(float ratio);

    vec3 getPos()       {return m_pos;}
    float getRotationX(){return m_rotX;}
    float getRotationY(){return m_rotY;}
    float getRotationZ(){return m_rotZ;}
    quat getRotation()  {return m_rotation;}
    float getFov()      {return m_fov;}
    float getAspectRatio(){return m_aspectRatio;}

    mat4 getViewProjectionMatrix(){return m_projMat*m_viewMat;}


    vec3 up();
    vec3 right();
    vec3 forward();


private:
    vec3 m_pos = {0, 0, 0};
    float m_rotX = 0;
    float m_rotY = 0;
    float m_rotZ = 0;
    float m_fov  = 90;
    float m_aspectRatio = 1;
    float m_nearClip    = 0.1f;
    float m_farClip     = 1000;

    quat m_rotation;
    mat4 m_viewMat;
    mat4 m_projMat;

    void calculateViewMat();
    void calculateProjMat();
};
