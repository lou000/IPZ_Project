﻿#pragma once
#include "../Core/math.h"

struct Plan
{
    glm::vec3 normal = { 0.f, 1.f, 0.f };
    float distance = 0.f;

    Plan() = default;
    Plan(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1))
    {}

    float getSignedDistanceToPlan(const glm::vec3& point) const
    {
        return glm::dot(normal, point) - distance;
    }
};

struct Frustum
{
    Plan topFace;
    Plan bottomFace;

    Plan rightFace;
    Plan leftFace;

    Plan farFace;
    Plan nearFace;
};

using namespace glm;
class Camera{
    friend class Serializer;

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

    vec3 getPos()           {return m_pos;}
    float getFov()          {return m_fov;}
    float getAspectRatio()  {return m_aspectRatio;}
    quat getRotation()      {return m_rotation;}
    float getNearClip()     {return m_nearClip;}
    float getFarClip()      {return m_farClip;}
    float getRotationX();
    float getRotationY();
    float getRotationZ();
    vec3 getMouseRay();
    Frustum getCameraFrustum();

    mat4 getViewMatrix();
    mat4 getProjMatrix();
    mat4 getViewProjectionMatrix();

    vec3 up();
    vec3 right();
    vec3 forward();

    virtual void onUpdate(float dt);
    virtual void onCreate();

protected:
    enum Type{
        Base,
        EditorCamera,
        GameCamera
    };


    //       SERIALIZED      //
    //-----------------------//
    Type type = Base;
    vec3 m_pos = {0, 0, 0};
    float m_fov  = 90;
    float m_aspectRatio = 1;
    float m_nearClip    = 0.1f;
    float m_farClip     = 1000;
    bool isActive = false;
    vec3 m_focusPoint = {0,0,0};
    quat m_rotation = {0,0,0,0};
    //------------------------//

private:
    mat4 m_viewMat;
    mat4 m_projMat;

    void updateViewMat();
    void updateProjMat();

    bool firstMouseClick = true; //temporary
};


