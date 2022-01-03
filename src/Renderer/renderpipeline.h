#pragma once
#include "../Core/scene.h"
#include <memory>

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);
    uint bloomDepth = 5;
    uvec2 winSize;
    FrameBuffer hdrFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    std::shared_ptr<Texture> whiteTexture;
    std::shared_ptr<Shader> screenShader;
    std::shared_ptr<Texture> bloomInputTex;
    std::shared_ptr<Texture> bloomOutputTex;
    VertexArray screenQuad;
};
