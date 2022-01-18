#pragma once
#include <chrono>
#include <unordered_map>
#include "glad/glad.h"

//TODO: Make all this thread safe
// All code below from https://github.com/TheCherno/Hazel, slightly modified

using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

struct ProfileResult
{
    std::string name;
    float ms;
    size_t threadID;
};

class InstrumentationStorage
{
    InstrumentationStorage(const InstrumentationStorage&) = delete;
    InstrumentationStorage(InstrumentationStorage&&) = delete;
    InstrumentationStorage(){}

public:
    static InstrumentationStorage& getInstance()
    {
        static InstrumentationStorage instance;
        return instance;
    }
    std::unordered_map<std::string, ProfileResult> storage;
};

using ProfileStorage = std::unordered_map<std::string, ProfileResult>;
class InstrumentationTimer
{
    ProfileResult m_result;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
    bool m_stopped;
    bool m_syncGPU;

public:

    InstrumentationTimer(const std::string & name, bool syncGPU = false)
        : m_stopped(false), m_syncGPU(syncGPU)
    {
        m_result.name = name;
        m_startTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~InstrumentationTimer()
    {
        if (!m_stopped) { stop(); }
    }

    void stop()
    {
        if(m_syncGPU)
            glFinish();
        auto endTimepoint = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch();

        m_result.ms = (float)elapsedTime.count()/1000;
        m_result.threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        std::string hash = m_result.name + std::to_string(m_result.threadID);

        auto& instance = InstrumentationStorage::getInstance();
        if(instance.storage.count(hash) == 0)
            instance.storage[hash] = m_result;
        if(instance.storage.count(hash) == 1)
            instance.storage[hash].ms += m_result.ms;
        m_stopped = true;
    }
};

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define _FUNCTION_ __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define _FUNCTION_ __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define _FUNCTION_ __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define _FUNCTION_ __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define _FUNCTION_ __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define _FUNCTION_ __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define _FUNCTION_ __func__
#else
#define _FUNCTION_ "_FUNCTION_ unknown!"
#endif

#define PROFILE_SCOPE_LINE2(name, line, synchronize) InstrumentationTimer timer##line(name, synchronize);
#define PROFILE_SCOPE_LINE(name, line, synchronize) PROFILE_SCOPE_LINE2(name, line, synchronize)
#define PROFILE_SCOPE(name) PROFILE_SCOPE_LINE(name, __LINE__, false)
#define PROFILE_FUNCTION() PROFILE_SCOPE(_FUNCTION_)
#define PROFILE_GPU(name, synchronize) PROFILE_SCOPE_LINE(name, __LINE__, synchronize)
