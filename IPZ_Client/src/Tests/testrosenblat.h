#pragma once
#include "../Core/application.h"


class TestRosenblat
{
    struct Point{
        vec2 pos;
        float val;
    };

public:
    TestRosenblat();
    void onUpdate(float dt);

private:
    vec2 winSize = App::getWindowSize();
    float margin = winSize.y /10;
    float gridSpacing = winSize.y / 10;
    float bottom = winSize.y - margin;
    float top    = margin;
    float right  = winSize.x - margin;
    float left   = margin;

    int nCenters = 100;
    int nPoints  = 1000;
    int k = 1;
    int maxK = 5000;
    float meshStep = 0.01f;
    float fi = 0.2f;
    float learnFactor = 0.05f;
    int trainPerFrame = 1;
    std::vector<Point> meshGrid;
    std::vector<Point> points;
    std::vector<vec2> centers;
    float* features;
    float* weights;
    vec2 rangeX, rangeY;

    void countour();
    float checkPoint(const Point &p);
    float calcFeature(vec2 point, vec2 center);
    int train(uint times);
};
