#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"
#include "../Renderer/graph.h"
#include "thread_pool.hpp"
#include <functional>

template<uint inputs, uint size>
class MLP
{
public:
    MLP(float learnFactor, std::function<float(float)> activationFunc);
    void reset();
    float predict(std::array<float, inputs> in);
    void train(std::array<float, inputs+1> in);

private:
    float m_learnFactor;
    std::function<float(float)> m_activationFunc;
    std::array<std::array<float, inputs+1>, size> weightsV;
    std::array<float, size+1> weightsW;
    thread_pool tPool = thread_pool();
};

class TestMLP : public Scene
{
public:
    TestMLP();
    ~TestMLP(){/*dontcare*/};
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;

private:
    vec3* points;
    vec3* pointsNormalized;
    vec3* points_nerfed;
    vec3* pointsNormalized_nerfed;
    float accum = 0;
    float lastLog1 = 0;
    float lastLog2 = 0;
    size_t prevMs1 = 1;
    std::atomic<size_t> prevMs2 = 1;
    uint trainCount1 = 0;
    uint trainCount2 = 0;
    thread_pool trainingThread1 = thread_pool(1);
    thread_pool trainingThread2 = thread_pool(1);
    uint trainLoopsPerAnim = 10;

    vec2 rangeXZ = {0.f, glm::pi<float>()};
    MLP<2, 80> mlp;
    MLP<2, 80> mlp_nerfed;
    Graph3d graphOrig = Graph3d(rangeXZ, {-1, 1}, rangeXZ, 10);
    Graph3d graphMLP  = Graph3d(rangeXZ, {-1, 1}, rangeXZ, 10);
    Graph3d graphMLP_nerfed  = Graph3d(rangeXZ, {-1, 1}, rangeXZ, 10);
};



