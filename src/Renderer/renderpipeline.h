#pragma once
#include "../Core/scene.h"
#include <memory>
#define BLOOM_SAMPLES 7

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);
    uvec2 winSize;
    FrameBuffer hdrFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    std::shared_ptr<Texture> whiteTexture;
    std::shared_ptr<Shader> screenShader;
    std::shared_ptr<Shader> downsampleAndBlur;
    std::shared_ptr<Shader> tentUpsampleAndAdd;
//    std::shared_ptr<Texture> bloomInputTex;
//    std::shared_ptr<Texture> bloomOutputTex;

    std::vector<std::shared_ptr<Texture>> downSampleTextures;
    std::vector<std::shared_ptr<Texture>> upSampleTextures;

    VertexArray screenQuad;
};
