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
    uvec2 oldWinSize;
    uint enabledPointLightCount = 0;

    //bloom
    uint bloomSamples = 0;
    float oldBloomRadius = 8;

    //CSM
    int oldShadowCascadeCount = 5;
    std::vector<float> cascadeRanges;
    int debugCascadeDisplayIndex = 0;
    float debugCascadeZextra = 10.f;
    float debugFirstCascadeOffset = 10.f;

    //SSAO
    std::vector<vec3> ssaoKernel;
    std::shared_ptr<Texture> ssaoNoiseTex;

    FrameBuffer hdrFBO;
    FrameBuffer csmFBO;
    FrameBuffer ssaoFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    StorageBuffer csmSSBO;
    StorageBuffer ssaoKernelSSBO;
    std::shared_ptr<Texture> whiteTexture;

    std::shared_ptr<Shader> pbrShader;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> screenShader;
    std::shared_ptr<Shader> blurMA;
    std::shared_ptr<Shader> downsampleAndBlur;
    std::shared_ptr<Shader> tentUpsampleAndAdd;
    std::shared_ptr<Shader> csmShader;
    std::shared_ptr<Texture> ssaoBlurTex;
    std::vector<std::shared_ptr<Texture>> bloomDownSampleTextures;
    std::vector<std::shared_ptr<Texture>> bloomUpSampleTextures;

    VertexArray screenQuad;
    void pbrPass(std::shared_ptr<Scene> scene, std::vector<Entity> entities);
    void CSMdepthPrePass(std::shared_ptr<Scene> scene, std::vector<Entity> entities);
    void bloomComputePass();
    void compositePass();
    void resizeOrClearResources();
    void maybeUpdateDynamicShaders(std::shared_ptr<Scene> scene);
    void drawSceneDebug(std::shared_ptr<Scene> scene);
    void drawScreenSpace(std::shared_ptr<Scene> scene);
    void drawImgui();
    void initSSAO();
    void ssaoPass(std::shared_ptr<Scene> scene);
    void ssaoBlur();
};
