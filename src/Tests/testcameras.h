#pragma once
#include "../Core/scene.h"
#include "../Core/application.h"
#include <opencv2/opencv.hpp>
#include "thread_pool.hpp"

class TestCameras : public Scene
{
private:
    cv::VideoCapture vcap;
    cv::Mat image;
    std::shared_ptr<Texture> texture;
    thread_pool pool;

public:
    TestCameras();
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;
};

