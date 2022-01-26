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
#define MAX_NOISE_OCTAVES 10
#define MAX_LIGHTS 200


struct PerlinOctave //TODO: move this
{
    alignas(16) vec4 frequency = {0,0,0,0};
    alignas(16) vec4 offset    = {0,0,0,0};
    float amplitude= 0;
    float padding = 0;
};

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
    int ssaoBlurSize = 4;
    float ssaoRadius = 0.6f;
    float ssaoBias = 0.2f;

    bool enableVolumetric = true;
    bool volumetricHalfRes = true;
    int volumetricSamples = 50;
    int volumetricBlurSize = 4;
    float g_factor = -0.1f;
    float fog_strength = 0.8f;
    float fog_y = 5;
    float lightShaftIntensity = 1.f;

    // TODO: move this
    std::vector<PerlinOctave> perlinOctavesFog;
    std::vector<PerlinOctave> perlinOctavesTerrain;


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
    bool showNoiseSettings = true;
    bool syncGPU = false;
    uvec2 winSize;
    uvec2 oldWinSize;
    int debugView = 0;

    // For framecount display
    float dtSum = 0;
    int frameCount = 0;
    float ms = 0;


    //PBR
    FrameBuffer hdrFBO;
    StorageBuffer lightsSSBO;
    StorageBuffer instanceTransformsSSBO;
    std::shared_ptr<Shader> pbrShader;
    std::shared_ptr<Texture> whiteTexture;
    uint enabledPointLightCount = 0;
    std::vector<Entity> pbrDrawGroup;
    std::vector<GPU_PointLight> lightsGroup;
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

    //Volumetric lights
    FrameBuffer vlFBO;
    FrameBuffer blurVlFBO;
    bool oldVlHalfRes = true;
    std::shared_ptr<Shader> vlShader;
    std::shared_ptr<Shader> upsampleVL;
    std::shared_ptr<Texture> upSampledVL;

    //SSAO
    FrameBuffer ssaoFBO;
    FrameBuffer blurSsaoFBO;
    StorageBuffer ssaoKernelSSBO;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> tentUpsampleDepth;
    std::shared_ptr<Shader> bilinearDownsample;
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

    //Noise
    StorageBuffer perlinOctavesFogSSBO;
    StorageBuffer perlinOctavesTerrainSSBO;
    std::shared_ptr<Shader> perlinNoiseGen;
    std::shared_ptr<Shader> terrainHeightShader;
    std::shared_ptr<Texture> perlinTextureFog;
    std::shared_ptr<Texture> perlinTextureTerrain;
    bool terrainMapChanged = true;
    void guiNoiseSettings();

    // MousePicking ????
    StorageBuffer mouseWorldPositionSSBO;
    std::shared_ptr<Shader> mousePositionShader;



    void pbrPass(std::shared_ptr<Scene> scene);
    void CSMdepthPrePass(std::shared_ptr<Scene> scene);
    void bloomComputePass();
    void downsampleDepth();
    void volumetricPass(std::shared_ptr<Scene> scene);
    void ssaoPass(std::shared_ptr<Scene> scene);
    void upsamplePass();
    void blurPass();
    void compositePass();

    void drawSceneDebug(std::shared_ptr<Scene> scene);
    void drawScreenSpace(std::shared_ptr<Scene> scene);
    void drawImgui(std::shared_ptr<Scene> scene);


    void resizeOrClearResources();
    void generateFogNoise();
    void maybeUpdateDynamicShaders(std::shared_ptr<Scene> scene);
    void initSSAO();
    void updateSSAOKernel();
    void generateTerrainNoise();
    void applyTerrainHeight();
    void getMouseWorldPosition(std::shared_ptr<Scene> scene);
};
