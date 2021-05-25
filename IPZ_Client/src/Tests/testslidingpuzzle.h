#pragma once
#include "SlidingPuzzle/slidepuzzle.h"
#include "../Core/application.h"
#include "../Core/scene.h"

class TestSlidingPuzzle : public Scene
{
public:
    TestSlidingPuzzle();
    ~TestSlidingPuzzle(){/*dontcare*/};
    virtual void onUpdate(float dt) override;
    virtual void onStart() override;
private:
    int n, size;
    float center;
    float animationSpeed = 2.f;
    float animProgress = 1.0f;
    SlidePuzzle* puzzle;
    informative_searcher* searcher;
    std::vector<const graph_state*> solutionPath;
    std::vector<const graph_state*>::iterator iter;
};

