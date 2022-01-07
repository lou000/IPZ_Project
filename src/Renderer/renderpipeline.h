#pragma once
#include "../Core/scene.h"
#include <memory>
#define MAX_BLOOM_SAMPLES 20
#define MAX_SHADOW_CASCADES 20

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);

private:
    void initFBOs();
    void initSSBOs();
    void initShaders();
    void resizeBloomBuffers();
    void updateSSBOs(std::shared_ptr<Scene> scene);
    void updateCascadeRanges();

    // tweakies
    float bloomRadius = 8;
    float bloomIntensity = 1;
    float bloomTreshold = 1;
    float exposure = 1;
    int shadowCascadeCount = 5;
    uint csmResolution = 4096;

    // no touchy
    uvec2 winSize;
    uint bloomSamples = 0;
    float oldBloomRadius = 8;
    int oldShadowCascadeCount = 5;
    std::vector<float> cascadeRanges;
    uint enabledPointLightCount = 0;

    int debugCascadeDisplayIndex = 0;
    float debugCascadeZextra = 10.f;
    float debugFirstCascadeOffset = 10.f;
    FrameBuffer hdrFBO;
    FrameBuffer csmFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    StorageBuffer csmSSBO;
    std::shared_ptr<Texture> whiteTexture;

    std::shared_ptr<Shader> pbrShader;
    std::shared_ptr<Shader> screenShader;
    std::shared_ptr<Shader> downsampleAndBlur;
    std::shared_ptr<Shader> tentUpsampleAndAdd;
    std::shared_ptr<Shader> csmShader;
    std::shared_ptr<Texture> cascadeTexture;
    std::vector<std::shared_ptr<Texture>> bloomDownSampleTextures;
    std::vector<std::shared_ptr<Texture>> bloomUpSampleTextures;

    VertexArray screenQuad;
};
