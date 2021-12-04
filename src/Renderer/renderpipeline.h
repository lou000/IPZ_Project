#pragma once
#include "../Core/scene.h"
#include <memory>

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);
    FrameBuffer mainFBO;
};
