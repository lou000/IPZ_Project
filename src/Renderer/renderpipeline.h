#pragma once
#include "../Core/scene.h"
#include <memory>

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);
    FrameBuffer hdrFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    std::shared_ptr<Texture> whiteTexture;
    std::shared_ptr<Shader> screenShader;
    VertexArray screenQuad;
};
