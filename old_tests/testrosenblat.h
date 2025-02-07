﻿#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"
#include "../Renderer/graph.h"
#include "thread_pool.hpp"


class TestRosenblat : public Scene
{
    struct Point{
        vec2 pos;
        float val;
    };

public:
    TestRosenblat();
    ~TestRosenblat(){/*dontcare*/};
    virtual void onUpdate(float dt) override;
    virtual void onStart() override;

private:

    int nCenters = 100;
    int nPoints  = 1000;
    int k = 1;
    int maxK = 8000;
    float meshStep = 0.01f;
    int meshX = (int)(2/meshStep);
    int meshSize = meshX*meshX;
    float fi = 0.2f;
    float learnFactor = 0.01f;
    int trainPerFrame = 1;
    Point* points;
    Point* meshGrid;
    thread_pool tPool = thread_pool(std::thread::hardware_concurrency()-1);
    vec2* centers;
    float* features;
    float* weights;
    vec2 rangeX, rangeY;
    Graph2d graph;

    void countour();
    int train(uint times);
    static float calcFeature(vec2 point, vec2 center, float fi);
};
