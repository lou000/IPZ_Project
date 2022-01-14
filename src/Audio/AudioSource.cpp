#include "AudioSource.h"
#include<vector>
#include<iostream>

AudioSource::AudioSource(std::shared_ptr<AudioBuffer> sFile)
    : sourceState(0), soundBuffer(sFile)
{
    alGenSources(1, &this->source);
    alSource3f(this->source, AL_POSITION, 1.f, 0.f, 0.f);
    alSource3f(this->source, AL_VELOCITY, 0.f, 0.f, 0.f);
    alSourcef(this->source, AL_PITCH, 1.f);
    alSourcef(this->source, AL_GAIN, 1.f);
    //alSourcef(this->source, AL_MAX_DISTANCE, 10000.f);
    alSourcei(this->source, AL_LOOPING, AL_FALSE);

    alSourcei(this->source, AL_BUFFER, soundBuffer->bufferID());
}

AudioSource::~AudioSource()
{
    alDeleteSources(1, &this->source);
}

void AudioSource::setLoop(bool loop)
{
    alSourcei(this->source, AL_LOOPING, (loop)?AL_TRUE:AL_FALSE);
}

void AudioSource::play()
{
    alSourcePlay(this->source);
}



void AudioSource::stop()
{
    alSourceStop(this->source);
}

void AudioSource::pause()
{
    alSourcePause(this->source);
}

void AudioSource::setPosition(vec3 pos)
{
    alSource3f(this->source, AL_POSITION, pos.x, pos.y, pos.z);
}
