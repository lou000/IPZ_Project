#pragma once
#include "../Core/scene.h"
#include "../AssetManagement/assets.h"
#include <memory>
#include "../Core/gui.h"
#define MAX_BLOOM_SAMPLES 20
#define MAX_SHADOW_CASCADES 20
#define MAX_SSAO_KERNEL_SIZE 256

struct RenderConfig
{
    bool enableBloom = true;
    float bloomRadius = 8;
    float bloomIntensity = 1;
    float bloomTreshold = 1;
    float exposure = 1;

    bool enableCSM = true;
    int shadowCascadeCount = 5;
    float cascadeZextra = 10.f;
    float firstCascadeOffset = 10.f;
    int csmResolution = 4096;

    bool enableSSAO = true;
    int ssaoKernelSize = 96;
    int blurKernelSize = 4;
    float ssaoRadius = 0.6f;
    float ssaoBias = 0.2f;

    int renderStatsCorner = 0;
};

class RenderPipeline
{
public:
    RenderPipeline();
    void drawScene(std::shared_ptr<Scene> scene);
    void serialize();

private:
    void initFBOs();
    void initSSBOs();
    void initShaders();
    void resizeBloomBuffers();
    void updateSSBOs(std::shared_ptr<Scene> scene);
    void updateCascadeRanges();


    // no touchy
    RenderConfig config;
    bool showRenderSettings = true;
    bool showRenderStats = true;
    bool syncGPU = false;
    uvec2 winSize;
    uvec2 oldWinSize;
    uint enabledPointLightCount = 0;

    //bloom
    uint bloomSamples = 0;
    float oldBloomRadius = 8;

    //CSM
    int oldShadowCascadeCount = 5;
    int oldCsmResolusion = 4096;
    std::vector<float> cascadeRanges;


    //SSAO
    int oldSsaoKernelSize = 64;
    std::vector<vec3> ssaoKernel;
    std::shared_ptr<Texture> ssaoNoiseTex;

    FrameBuffer hdrFBO;
    FrameBuffer csmFBO;
    FrameBuffer ssaoFBO;
    FrameBuffer blurFBO;
    FrameBuffer outputFBO;
    StorageBuffer lightsSSBO;
    StorageBuffer csmSSBO;
    StorageBuffer ssaoKernelSSBO;
    std::shared_ptr<Texture> whiteTexture;

    std::shared_ptr<Shader> pbrShader;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> screenShader;
    std::shared_ptr<Shader> simpleBlur;
    std::shared_ptr<Shader> downsampleAndBlur;
    std::shared_ptr<Shader> tentUpsampleAndAdd;
    std::shared_ptr<Shader> csmShader;
    std::shared_ptr<Shader> solidShader;
    std::vector<std::shared_ptr<Texture>> bloomDownSampleTextures;
    std::vector<std::shared_ptr<Texture>> bloomUpSampleTextures;

    VertexArray screenQuad;
    void pbrPass(std::shared_ptr<Scene> scene);
    void CSMdepthPrePass(std::shared_ptr<Scene> scene);
    void bloomComputePass();
    void compositePass();
    void resizeOrClearResources();
    void maybeUpdateDynamicShaders(std::shared_ptr<Scene> scene);
    void drawSceneDebug(std::shared_ptr<Scene> scene);
    void drawScreenSpace(std::shared_ptr<Scene> scene);
    void drawImgui(std::shared_ptr<Scene> scene);
    void initSSAO();
    void ssaoPass(std::shared_ptr<Scene> scene);
    void ssaoBlur();
    void updateSSAOKernel();
};
