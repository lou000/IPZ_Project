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

    reset();
}

template<uint inputs, uint size>
void MLP<inputs, size>::reset()
{
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
float MLP<inputs, size>::predict(std::array<float, inputs> in)
{
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
void MLP<inputs, size>::train(std::array<float, inputs+1> in)
{
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

    float prediction = 0;
    for(uint n=0; n<size; n++)
        prediction += firstLayerSums[n]*weightsW[n];
    prediction += weightsW[size];

    float label      = in[inputs];
    float diff = prediction - label;

    for(uint n=0; n<size; n++)
    {
        auto& nWeights = weightsV[n];
        auto s = firstLayerSums[n];
        for(uint i=0; i<inputs; i++)
            nWeights[i]  -= m_learnFactor*diff*weightsW[n]*s*(1-s)*in[i];
        nWeights[inputs] -= m_learnFactor*diff*weightsW[n]*s*(1-s);
    }

    for(uint n=0; n<size; n++)
        weightsW[n] -= m_learnFactor*diff*firstLayerSums[n];
    weightsW[size]   -= m_learnFactor*diff;
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
    pointsNormalized = (vec3*)malloc(POINT_COUNT*sizeof(vec3));
    for(uint i=0; i<POINT_COUNT; i++)
    {
        pointsNormalized[i].x = mapToRange(rangeXZ, {-1,1}, points[i].x);
        pointsNormalized[i].z = mapToRange(rangeXZ, {-1,1}, points[i].z);
        pointsNormalized[i].y = points[i].y;
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
    if(App::getKeyOnce(GLFW_KEY_KP_SUBTRACT))
        trainLoopsPerAnim -= 10;
    if(App::getKeyOnce(GLFW_KEY_KP_ADD))
        trainLoopsPerAnim += 10;
    if(App::getKeyOnce(GLFW_KEY_SPACE))
        mlp.reset();

    float animSpeed = 1.0f;//(float)prevMs/1000;
    if(trainingThread.get_tasks_running() == 0 && !graphMLP.animating())
    {
        graphMLP.animateTo([=](const vec2& p){
            float x = mapToRange(rangeXZ, {-1,1}, p.x);
            float z = mapToRange(rangeXZ, {-1,1}, p.y);
            return mlp.predict({x, z});
        }, animSpeed);
        trainingThread.submit([&](){
            timer t;
            t.start();
            for(uint x=0; x<trainLoopsPerAnim; x++)
            {
                for(uint i=0; i<POINT_COUNT; i++)
                {
                    mlp.train({pointsNormalized[i].x, pointsNormalized[i].z, pointsNormalized[i].y});
                    trainCount++;
                }
            }
            t.stop();
            prevMs = t.ms();
            LOG("count: %d, ms: %lld", trainCount, prevMs);
        });
    }

    graphOrig.draw({0,0,0}, dt);
    graphMLP.draw({15,0,0}, dt);

}
