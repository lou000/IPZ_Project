#include "AudioListener.h"


AudioListener::AudioListener()
{
	const ALCchar* defaultDeviceString = alcGetString(/*device*/nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	device = alcOpenDevice(defaultDeviceString);
	context = alcCreateContext(device, nullptr);
	alcMakeContextCurrent(context);
	//{
	//	std::cerr << "failed to make the OpenAL context the current context" << std::endl;
	//	return -1;
	//}
	alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
	alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
	ALfloat forwardAndUpVectors[] = {
		/*forward = */ 1.f, 0.f, 0.f,
		/* up = */ 0.f, 1.f, 0.f
	};
	alListenerfv(AL_ORIENTATION, forwardAndUpVectors);
}

AudioListener::~AudioListener()
{
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void AudioListener::setPosition(vec3 pos)
{
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
}

void AudioListener::setOrientation(vec3 cameraForward, vec3 cameraUp)
{
    vec3 arr[] = {cameraForward, cameraUp};
    alListenerfv(AL_ORIENTATION, (ALfloat*) arr);
}
