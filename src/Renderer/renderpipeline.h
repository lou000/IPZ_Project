#pragma once
#include "../Core/scene.h"
#include <memory>
#define MAX_BLOOM_SAMPLES 20

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);

private:

    // tweakies
    float bloomRadius = 8;
    float bloomIntensity = 1;
    float bloomTreshold = 1;
    float exposure = 1;

    // no touchy
    void resizeBloomBuffers();

    uvec2 winSize;
    uint bloomSamples = 0;
    float oldBloomRadius = 8;
    FrameBuffer hdrFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    std::shared_ptr<Texture> whiteTexture;
    std::shared_ptr<Shader> screenShader;
    std::shared_ptr<Shader> downsampleAndBlur;
    std::shared_ptr<Shader> tentUpsampleAndAdd;
    std::vector<std::shared_ptr<Texture>> downSampleTextures;
    std::vector<std::shared_ptr<Texture>> upSampleTextures;

    VertexArray screenQuad;

};
