#pragma once
#include "slidepuzzle.h"
#include "application.h"

class TestSlidingPuzzle
{
public:
    TestSlidingPuzzle();
    void onUpdate(float dt);
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

