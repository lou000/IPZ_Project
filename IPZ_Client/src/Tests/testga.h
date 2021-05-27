#pragma once

#include "../Core/application.h"
#include "../Core/scene.h"
#include "../Renderer/graph.h"
#include <vector>
#include <functional>

class GA
{
public:
    GA(uint popCount, uint genCount, std::function<float(uint*, uint)> fitnessFunc);

    //                                     genes genCount
    void initPopulation(std::function<void(uint*, uint)> initFunc);

    //                      selIndex pop popCount genCount
    void select(std::function<uint(uint*, uint, uint)> selectFunc);

    //                            genes1 genes2 genCount
    void cross(std::function<void(uint*, uint*, uint)> crossFunc);

    //                             genes  genCount
    void mutate(std::function<void(uint*, uint)> mutateFunc);

private:
    uint*  m_population;
    float* m_fitnessScores;
    uint   m_popCount;
    uint   m_genCount;


};

class TestGA : public Scene
{
public:
    TestGA();
    ~TestGA(){/*dontcare*/};
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;
};
