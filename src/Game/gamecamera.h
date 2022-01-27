#pragma once
#include "../Renderer/camera.h"

class GameCamera : public Camera
{
public:
    GameCamera(float fov, float aspectRatio, float nearClip, float farClip)
        : Camera(fov, aspectRatio, nearClip, farClip){}

    virtual void onUpdate(float dt) override;
    virtual void onCreate() override;
    void doTheFunkyThing(float dt);
    void animateMove(vec3 targetPos);

private:
    float timeFullSpeed = 0.5f;
    float timeAccelerating = 0.3f;
    float desiredSpeed = 0.f;
    float acceleration = 0.f;
    float currentSpeed = 0.f;
    bool animating = false;
    vec3 originalPos = {0,0,0};
    vec3 targetPos = {0,0,0};

};

