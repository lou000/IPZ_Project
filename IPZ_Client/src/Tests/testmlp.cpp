#include "testmlp.h"
#include "gtc/random.hpp"
#define MESH_X 40
#define POINT_COUNT 1000
#define START_TIMED_BLOCK() \
    timer __t;   \
    __t.start()  \

#define STOP_TIMED_BLOCK(x) \
    __t.stop(); \
    LOG("%s : %lld ms", x, __t.ms()) \


template<uint inputs, uint size>
MLP<inputs, size>::MLP(float learnFactor, std::function<float (float)> activationFunc)
{
    m_learnFactor = learnFactor;
    m_activationFunc = activationFunc;

    for(uint i=0; i<size; i++)
    {
        for(uint j=0; j<inputs+1; j++)
        {
            weightsV[i][j] = glm::linearRand(-0.001f, 0.001f);
        }
    }

    for(uint j=0; j<size+1; j++)
    {
        weightsW[j] = glm::linearRand(-0.001f, 0.001f);
    }
}

template<uint inputs, uint size>
float MLP<inputs, size>::predict(const float* in, uint inSize)
{
    ASSERT(inSize/sizeof(float) == inputs);
    // First layer
    std::array<float, size> firstLayerSums;
    firstLayerSums.fill(0);
    for(uint n=0; n<size; n++)
    {
        auto nWeights = weightsV[n];
        for(uint i=0; i<inputs; i++)
            firstLayerSums[n]+=in[i]*nWeights[i];
        firstLayerSums[n]+=nWeights[inputs];
        firstLayerSums[n] = m_activationFunc(firstLayerSums[n]);
    }

    // Second layer
    float output = 0;
    for(uint n=0; n<size; n++)
        output += firstLayerSums[n]*weightsW[n];
    output += weightsW[size];
    return output;
}

template<uint inputs, uint size>
uint MLP<inputs, size>::train(float* dataPoints, uint dataSize)
{
    ASSERT((dataSize/sizeof(float)) % (inputs+1) == 0);
    uint pointCount = (dataSize/(inputs+1))/sizeof(float);

    std::vector<std::pair<float, float*>> wrongOutputs;
    for(uint i=0; i<pointCount; i++)
    {
        uint offset = i*(inputs+1);
        float prediction = predict(dataPoints+offset, inputs*sizeof(float));
        float label      = (dataPoints+offset)[inputs];
        float diff = prediction - label;
        if(abs(diff) > 0.001)
            wrongOutputs.push_back({diff, dataPoints+offset});
    }


    auto randOut = wrongOutputs[glm::linearRand<uint>(0, (uint)wrongOutputs.size()-1)];
    float randDiff   = randOut.first;
    float* randInput = randOut.second;

    // Recalculate and correct layers... maybe cache them?
    std::array<float, size> firstLayerSums;
    firstLayerSums.fill(0);
    for(uint n=0; n<size; n++)
    {
        auto nWeights = weightsV[n];
        for(uint i=0; i<inputs; i++)
            firstLayerSums[n]+=randInput[i]*nWeights[i];
        firstLayerSums[n]+=nWeights[inputs];
        firstLayerSums[n] = m_activationFunc(firstLayerSums[n]);
    }

    for(uint n=0; n<size; n++)
    {
        auto& nWeights = weightsV[n];
        auto s = firstLayerSums[n];
        for(uint i=0; i<inputs; i++)
            nWeights[i]  -= m_learnFactor*randDiff*weightsW[n]*s*(1-s)*randInput[i];
        nWeights[inputs] -= m_learnFactor*randDiff*weightsW[n]*s*(1-s);
    }

    for(uint n=0; n<size; n++)
        weightsW[n] -= m_learnFactor*randDiff*firstLayerSums[n];
    weightsW[size]   -= m_learnFactor*randDiff;

    return (uint) wrongOutputs.size();
}


TestMLP::TestMLP()
    : mlp(0.07f, [](float n){return 1/(1+exp(-n));})
{
    BatchRenderer::setShader(AssetManager::getShader("batch"));
    MeshRenderer::setShader(AssetManager::getShader("mesh"));

    points = (vec3*)malloc(POINT_COUNT*sizeof(vec3));
    // points
    for(uint i=0; i<POINT_COUNT; i++)
    {
        auto& p = points[i];
        p.x = glm::linearRand(rangeXZ.x, rangeXZ.y);
        p.z = glm::linearRand(rangeXZ.x, rangeXZ.y);
        p.y = cos(p.x*p.z)*cos(2*p.x);
    }
    pointsNormalized = (float*)malloc(POINT_COUNT*3*sizeof(float));
    for(uint i=0; i<POINT_COUNT; i++)
    {
        pointsNormalized[i*3+0] = mapToRange(rangeXZ, {-1,1}, points[i].x);
        pointsNormalized[i*3+1] = mapToRange(rangeXZ, {-1,1}, points[i].z);
        pointsNormalized[i*3+2] = points[i].y;
    }


    graphOrig.setMesh(MESH_X, MESH_X, false);
    graphOrig.updateMesh([=](const vec2& p){
        return cos(p.x*p.y)*cos(2*p.x);
    });
    graphOrig.setPoints(points, POINT_COUNT);

    graphMLP.setMesh(MESH_X, MESH_X, false);
    graphMLP.setPoints(points, POINT_COUNT);
}

void TestMLP::onUpdate(float dt)
{
    GraphicsContext::getCamera()->setFocusPoint({12.5f,5,5});

    if(trainingThread.get_tasks_running() == 0)
    {
        graphMLP.animateTo([=](const vec2& p){
            vec2 temp = {mapToRange(rangeXZ, {-1,1}, p.x), mapToRange(rangeXZ, {-1,1}, p.y)};
            return mlp.predict(glm::value_ptr(temp), sizeof(vec2));
        }, (float)prevMs/1000);
        trainingThread.submit([&](){
            timer t;
            t.start();
            for(uint i=0; i<500; i++)
            {
                mlp.train(pointsNormalized, POINT_COUNT*3*sizeof (float));
                trainCount++;
            }
            t.stop();
            prevMs = t.ms();
            LOG("count: %d, ms: %lld", trainCount, prevMs);
        });
    }

    graphOrig.draw({0,0,0}, dt);
    graphMLP.draw({15,0,0}, dt);

}
