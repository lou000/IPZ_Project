#include "testmlp.h"
#include "gtc/random.hpp"
#define MESH_X 40
#define POINT_COUNT 1000
#define POINT_COUNT_NERFED 100
#define NERFED_TEST_COUNT 30
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
    : mlp(       0.02f, [](float n){return 1/(1+exp(-n));}),
      mlp_nerfed(0.02f, [](float n){return 1/(1+exp(-n));})
{
    BatchRenderer::setShader(AssetManager::getShader("batch"));
    MeshRenderer::setShader(AssetManager::getShader("mesh"));

    // points
    points = (vec3*)malloc(POINT_COUNT*sizeof(vec3));
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

    // points nerfed
    points_nerfed = (vec3*)malloc(POINT_COUNT_NERFED*sizeof(vec3));
    for(uint i=0; i<POINT_COUNT_NERFED; i++)
    {
        auto& p = points_nerfed[i];
        p.x = glm::linearRand(rangeXZ.x, rangeXZ.y);
        p.z = glm::linearRand(rangeXZ.x, rangeXZ.y);
        p.y = cos(p.x*p.z)*cos(2*p.x);// + glm::linearRand(-0.2f, 0.2f);
    }
    pointsNormalized_nerfed = (vec3*)malloc(POINT_COUNT_NERFED*sizeof(vec3));
    for(uint i=0; i<POINT_COUNT_NERFED; i++)
    {
        pointsNormalized_nerfed[i].x = mapToRange(rangeXZ, {-1,1}, points_nerfed[i].x);
        pointsNormalized_nerfed[i].z = mapToRange(rangeXZ, {-1,1}, points_nerfed[i].z);
        pointsNormalized_nerfed[i].y = points_nerfed[i].y;
    }


    graphOrig.setMesh(MESH_X, MESH_X, false);
    graphOrig.setPaletteBlend({{0.267, 0.004, 0.329, 1},{0.118, 0.624, 0.533,1},{0.992, 0.906, 0.141,1}}, 200);
    graphOrig.updateMesh([=](const vec2& p){
        return cos(p.x*p.y)*cos(2*p.x);
    });
    graphOrig.addPoints(points, POINT_COUNT, {0.2f,0.2f,0.6f,1});

    graphMLP.setMesh(MESH_X, MESH_X, false);
    graphMLP.addPoints(points, POINT_COUNT, {0.2f,0.2f,0.6f,1});

    graphMLP_nerfed.setMesh(MESH_X, MESH_X, false);
    graphMLP_nerfed.setPaletteBlend({{0.941, 0.906, 0.941, 1},{0.729, 0.400, 0.337,1},{0.157, 0.031, 0.176,1},{0.416, 0.545, 0.749,1},{0.929, 0.902, 0.937,1}}, 200);
    graphMLP_nerfed.addPoints(points_nerfed, POINT_COUNT_NERFED-NERFED_TEST_COUNT, {0.2f,0.2f,0.6f,1});
    graphMLP_nerfed.addPoints(points_nerfed+POINT_COUNT_NERFED-NERFED_TEST_COUNT, NERFED_TEST_COUNT, {1,0,0,1});
}

void TestMLP::onStart()
{
    GraphicsContext::setClearColor({0.184f, 0.200f, 0.329f, 1.f});
    auto camera = GraphicsContext::getCamera();
    camera->setFov(50.f);
    camera->setPosition({20.f,15.f,40.f});
    camera->setFocusPoint({20.f,5,5});
}

void TestMLP::onUpdate(float dt)
{
    accum+=dt;
    if(App::getKeyOnce(GLFW_KEY_KP_SUBTRACT))
        trainLoopsPerAnim -= 10;
    if(App::getKeyOnce(GLFW_KEY_KP_ADD))
        trainLoopsPerAnim += 10;
    if(App::getKeyOnce(GLFW_KEY_SPACE))
    {
        mlp.reset();
        mlp_nerfed.reset();
        trainCount1 = 0;
        trainCount2 = 0;
    }

    float animSpeed1 = (float)(prevMs1+1)/1000;
    float animSpeed2 = (float)(prevMs2+1)/1000;

    if(trainingThread1.get_tasks_total() == 0 && !graphMLP.animating())
    {
        graphMLP.animateTo([=](const vec2& p){
            float x = mapToRange(rangeXZ, {-1,1}, p.x);
            float z = mapToRange(rangeXZ, {-1,1}, p.y);
            return mlp.predict({x, z});
        }, animSpeed1);

        uint errorLearnSet = 0;
        for(uint i=0; i<POINT_COUNT_NERFED-NERFED_TEST_COUNT; i++)
        {
            auto p = pointsNormalized[i];
            float pred = mlp.predict({p.x, p.z});
            if(abs(p.y-pred)>0.05)
                errorLearnSet++;
        }
        if(accum-lastLog1 > 5.f)
        {
            lastLog1 = accum;
            LOG("[MLP]  trainCount: %d, errorLearnSet: %d",
                trainCount1, errorLearnSet);
        }

        trainingThread1.submit([&](){
            timer t;
            t.start();
            for(uint x=0; x<trainLoopsPerAnim; x++)
                for(uint i=0; i<POINT_COUNT; i++)
                {
                    mlp.train({pointsNormalized[i].x, pointsNormalized[i].z, pointsNormalized[i].y});
                    trainCount1++;
                }

            t.stop();
            prevMs1 = t.ms();
        });
    }

    if(trainingThread2.get_tasks_total() == 0 && !graphMLP_nerfed.animating())
    {
        graphMLP_nerfed.animateTo([=](const vec2& p){
            float x = mapToRange(rangeXZ, {-1,1}, p.x);
            float z = mapToRange(rangeXZ, {-1,1}, p.y);
            return mlp_nerfed.predict({x, z});
        }, animSpeed2);

        uint errorLearnSet = 0;
        for(uint i=0; i<POINT_COUNT_NERFED-NERFED_TEST_COUNT; i++)
        {
            auto p = pointsNormalized_nerfed[i];
            float pred = mlp_nerfed.predict({p.x, p.z});
            if(abs(p.y-pred)>0.05)
                errorLearnSet++;
        }
        uint errorTestSet = 0;
        for(uint i=POINT_COUNT_NERFED-NERFED_TEST_COUNT; i<POINT_COUNT_NERFED; i++)
        {
            auto p = pointsNormalized_nerfed[i];
            float pred = mlp_nerfed.predict({p.x, p.z});
            if(abs(p.y-pred)>0.05)
                errorTestSet++;
        }
        if(accum-lastLog2 > 5.f)
        {
            lastLog2 = accum;
            LOG("[Nerfed MLP]  trainCount: %d, errorLearnSet: %d, errorTestSet: %d",
                trainCount2, errorLearnSet, errorTestSet);
        }

        trainingThread2.submit([&](){
            timer t;
            t.start();
            for(uint x=0; x<trainLoopsPerAnim; x++)
                for(uint i=0; i<POINT_COUNT_NERFED-NERFED_TEST_COUNT; i++)
                {
                    mlp_nerfed.train({pointsNormalized_nerfed[i].x,
                                      pointsNormalized_nerfed[i].z,
                                      pointsNormalized_nerfed[i].y});
                    trainCount2++;
                }
            t.stop();
            prevMs2 = t.ms();
        });
    }

    graphOrig.draw(      { 0,0,0}, dt);
    graphMLP.draw(       {15,0,0}, dt);
    graphMLP_nerfed.draw({30,0,0}, dt);

}


