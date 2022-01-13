#pragma once
#include<string>
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>
//Mono sound class
class AudioSource
{
public:
	AudioSource(std::string path);
	~AudioSource();
	void setLoop(bool loop);
	void SetGain(float gain){ alSourcef(this->source, AL_GAIN,gain); }
	void Play();
	void Stop();
	void Pause();

	bool IsPlaying() const { 
		alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		return sourceState == AL_PLAYING; }

	void UpdatePosition(float x, float y, float z);
	
private:
	ALuint soundBuffer;
	ALuint source;
	mutable ALint sourceState;
};

