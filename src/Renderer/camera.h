#pragma once
#include "../Core/utilities.h"

using namespace glm;
class Camera{

public:
    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    // Most of this functionality should be moved to entity base class class

    void move(vec3 vec);
    void setPosition(vec3 pos){m_pos = pos;}
    void setFov(float fov){m_fov = fov;}
    void setAspectRatio(float ratio){m_aspectRatio = ratio;}
    void setRotation(quat rotation){m_rotation = rotation;}
    void setRotationX(float degree);
    void setRotationY(float degree);
    void setRotationZ(float degree);
    void addRotationX(float degree);
    void addRotationY(float degree);
    void addRotationZ(float degree);
    void setFocusPoint(vec3 point){m_focusPoint = point; pointAt(point);}
    void pointAt(vec3 pos);

    vec3 getPos()          {return m_pos;}
    float getFov()         {return m_fov;}
    float getAspectRatio(){return m_aspectRatio;}
    quat getRotation()    {return m_rotation;}
    float getRotationX();
    float getRotationY();
    float getRotationZ();
    vec3 getMouseRay();

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
    bool isActive = false;
    vec3 m_focusPoint = {0,0,0};

    quat m_rotation = {1,0,0,0};
    mat4 m_viewMat;
    mat4 m_projMat;

    void updateViewMat();
    void updateProjMat();

    bool firstMouseClick = true; //temporary
};
