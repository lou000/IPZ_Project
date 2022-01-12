#include "AudioSource.h"
#define DR_WAV_IMPLEMENTATION
#include<dr_wav.h>
#include<vector>
#include<iostream>
#include<stdexcept>

struct ReadWavData
{
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    drwav_uint64 totalPCMFrameCount = 0;
    std::vector<uint16_t> pcmData;
    drwav_uint64 getTotalSamples() { return totalPCMFrameCount * channels; }
};

AudioSource::AudioSource(std::string path) : sourceState(0)
{
    ReadWavData monoData;
    drwav_int16* const pSampleData = drwav_open_file_and_read_pcm_frames_s16(path.c_str(), &monoData.channels, &monoData.sampleRate, &monoData.totalPCMFrameCount, nullptr);
    if (pSampleData == NULL) {
        std::cerr << "failed to load audio file\n";
        drwav_free(pSampleData, nullptr); //todo use raii to clean this up
        throw new std::ios_base::failure("failed to load audio file");
    }
    if (monoData.getTotalSamples() > drwav_uint64(std::numeric_limits<size_t>::max()))
    {
        drwav_free(pSampleData, nullptr);
        throw new std::length_error("too much data in file for 32bit addressed vector");
    }
    monoData.pcmData.resize(size_t(monoData.getTotalSamples()));
    std::memcpy(monoData.pcmData.data(), pSampleData, monoData.pcmData.size() * /*twobytes_in_s16*/2);
    drwav_free(pSampleData, nullptr);
    alGenBuffers(1, &this->soundBuffer);
    alBufferData(this->soundBuffer, monoData.channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, monoData.pcmData.data(), monoData.pcmData.size() * 2 /*two bytes per sample*/, monoData.sampleRate);
    alGenSources(1, &this->source);
    alSource3f(this->source, AL_POSITION, 1.f, 0.f, 0.f);
    alSource3f(this->source, AL_VELOCITY, 0.f, 0.f, 0.f);
    alSourcef(this->source, AL_PITCH, 1.f);
    alSourcef(this->source, AL_GAIN, 1.f);
    //alSourcef(this->source, AL_MAX_DISTANCE, 10000.f);
    alSourcei(this->source, AL_LOOPING, AL_FALSE);

    alSourcei(this->source, AL_BUFFER, this->soundBuffer);
}

AudioSource::~AudioSource()
{
    alDeleteBuffers(1, &this->soundBuffer);
    alDeleteSources(1, &this->source);
}

void AudioSource::setLoop(bool loop)
{
    alSourcei(this->source, AL_LOOPING, (loop)?AL_TRUE:AL_FALSE);
}

void AudioSource::Play()
{
    alSourcePlay(this->source);
}



void AudioSource::Stop()
{
    alSourceStop(this->source);
}

void AudioSource::Pause()
{
    alSourcePause(this->source);
}

void AudioSource::UpdatePosition(float x, float y, float z)
{
    alSource3f(this->source, AL_POSITION, x, y, z);
}
