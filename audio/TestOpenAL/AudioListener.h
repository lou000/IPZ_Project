#pragma once
#include <AL/al.h>
#include <AL/alc.h>
class AudioListener
{
public:
	AudioListener();

	~AudioListener();

	void UpdatePosition(float x, float y, float z);

private:
	ALCdevice* device;
	ALCcontext* context;
};

