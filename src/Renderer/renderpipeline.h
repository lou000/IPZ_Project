#pragma once
#include "../Core/scene.h"
#include "../AssetManagement/assets.h"
#include <memory>
#include "../Core/gui.h"
#include "../Core/entity.h"

#define MAX_BLOOM_SAMPLES 20
#define MAX_SHADOW_CASCADES 20
#define MAX_INSTANCED 10000
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
    bool ssaoHalfRes = true;
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


    //PBR
    FrameBuffer hdrFBO;
    StorageBuffer lightsSSBO;
    StorageBuffer instanceTransformsSSBO;
    std::shared_ptr<Shader> pbrShader;
    std::shared_ptr<Texture> whiteTexture;
    uint enabledPointLightCount = 0;
    std::unordered_map<int, std::vector<Entity>> instancedGroups;
    std::unordered_map<int, std::vector<mat4>> instancedTransforms;

    //Bloom
    std::shared_ptr<Shader> bloomDownsample;
    std::shared_ptr<Shader> bloomUpsample;
    std::vector<std::shared_ptr<Texture>> bloomDownSampleTextures;
    std::vector<std::shared_ptr<Texture>> bloomUpSampleTextures;
    uint bloomSamples = 0;
    float oldBloomRadius = 8;

    //CSM
    FrameBuffer csmFBO;
    StorageBuffer csmSSBO;
    std::shared_ptr<Shader> csmShader;
    int oldShadowCascadeCount = 5;
    int oldCsmResolusion = 4096;
    std::vector<float> cascadeRanges;

    //SSAO
    FrameBuffer ssaoFBO;
    FrameBuffer blurFBO;
    StorageBuffer ssaoKernelSSBO;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> bilinearDownsample;
    std::shared_ptr<Shader> tentUpsample;
    std::shared_ptr<Shader> simpleBlur;
    std::shared_ptr<Texture> ssaoNoiseTex;
    std::shared_ptr<Texture> downSampledDepth;
    std::shared_ptr<Texture> upSampledSSAO;
    int oldSsaoKernelSize = 64;
    bool oldSsaoHalfRes = true;
    std::vector<vec3> ssaoKernel;

    //Composite
    FrameBuffer outputFBO;
    std::shared_ptr<Shader> screenShader;
    VertexArray screenQuad;

    //Solid
    std::shared_ptr<Shader> solidShader;



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
