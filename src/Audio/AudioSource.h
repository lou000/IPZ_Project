#pragma once
#include<string>
#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>
#include "../Core/math.h"
#include "../Core/utilities.h"
#include "../AssetManagement/assets.h"
//Mono sound class
class AudioSource
{
public:
    AudioSource(std::shared_ptr<AudioBuffer> sFile);
	~AudioSource();
	void setLoop(bool loop);
    void setGain(float gain){ alSourcef(this->source, AL_GAIN,gain); }
    void play();
    void stop();
    void pause();

    bool isPlaying() const {
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		return sourceState == AL_PLAYING; }

    void setPosition(vec3 pos);

private:
	ALuint source;
	mutable ALint sourceState;
    std::shared_ptr<AudioBuffer> soundBuffer;
};

