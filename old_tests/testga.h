#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"
#include "../Renderer/graph.h"
#include <vector>
#include <functional>

template<typename T>
class GA
{
public:
    ~GA();
    GA(uint popCount, uint geneCount, float mutateChance, float crossChance);

    //                                     genes geneCount
    void initPopulation(std::function<void(T*, uint)> initFunc);

    //                     selIndex fitness popCount
    void select(std::function<uint(float*, uint)> selectFunc);

    //                            genes1 genes2 geneCount
    void cross(std::function<void(T*, T*, uint)> crossFunc);

    //                             genes  geneCount
    void mutate(std::function<void(T*, uint)> mutateFunc);

    //                          fitness genes geneCount
    void evaluate(std::function<float(T*, uint)> fitnessFunc);

    std::pair<T*, float> best();

    float avg(){return m_avgScore;}

private:
    T*  m_population;
    float* m_fitnessScores;
    float* m_scoresNormalized;
    uint   m_popCount;
    uint   m_geneCount;
    float  m_avgScore;

    float m_mutateChance;
    float m_crossChance;
};

class TestGA : public Scene
{
public:
    TestGA();
    ~TestGA(){/*dontcare*/};
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;

private:
    GA<uint>* ga;
    vec2* cities;
    uint* indexes;
    std::function<float(uint*, uint)> fitFunc;


    float timePassed = 0;
    float prevLog = 0;
    float bestScore = -std::numeric_limits<float>::infinity();

    vec2 prevAvg         = {0,0};
    vec2 prevBestGen     = {0,0};
    vec2 prevBestOverall = {0,0};


    Graph2d allTimeBest;
    Graph2d currentBest;
    Graph2d avgPopScore;
    Graph2d bestPerGen;
    Graph2d bestOverall;
};
