#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include "../Core/math.h"
class AudioListener
{
public:
    AudioListener();

    ~AudioListener();

    void setPosition(vec3 pos);
    void setOrientation(vec3 cameraForward, vec3 cameraUp);

private:
    ALCdevice* device;
    ALCcontext* context;
};

