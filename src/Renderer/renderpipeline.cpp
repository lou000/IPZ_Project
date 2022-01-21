#include "renderpipeline.h"
#include"../Core/yamlserialization.h"
#include "../Core/application.h"
#include "../Core/entity.h"
#include "../Core/components.h"
#include "../Core/instrumentation.h"
#include <random>


RenderPipeline::RenderPipeline()
{
    winSize = App::getWindowSize();
    oldWinSize = winSize;

    config = Serializer::deserializeRenderConfig("../Config/render_config.pc");

    initShaders();
    initFBOs();
    initSSAO();
    initSSBOs();
    updateCascadeRanges();
    // Initialize bloom buffers
    resizeBloomBuffers();

    perlinTextureFog = std::make_shared<Texture>(512, 512, 64, GL_R16F, GL_REPEAT);
    perlinTextureTerrain = std::make_shared<Texture>(512, 512, 64, GL_R16F, GL_REPEAT);
    generateNoise();


    // TODO: move everything below this to primitive generation
    //---------------------------------------------------------------//

    // Create white texture for non-textured drawing
    whiteTexture = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTexture->setData(&whiteData, sizeof(uint));

    // Create VOA for a quad TODO: move this to primitive generation
    const float quadVertices[24] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    BufferLayout layout = {
        {BufferElement::Float2, "a_Position" },
        {BufferElement::Float2, "a_TexCoord" }
    };
    auto vbo = std::make_shared<VertexBuffer>(layout, sizeof(quadVertices), (void*)&quadVertices);
    screenQuad.addVBuffer(vbo);

    //---------------------------------------------------------------//

}


void RenderPipeline::drawScene(std::shared_ptr<Scene> scene)
{
    oldWinSize = winSize;
    winSize = App::getWindowSize();

    // gather instanced entities
    instancedGroups.clear();
    auto group = scene->entities().group<TransformComponent, MeshComponent, RenderSpecComponent, InstancedDrawComponent>();
    for(auto& ent : group)
    {
        auto& spec = group.get<InstancedDrawComponent>(ent);
        instancedGroups[spec.instancedGroup].push_back(scene->fromEntID(ent));
    }

    // gather the transforms
    for(auto& group : instancedGroups)
    {
        std::vector<mat4> groupTransforms;
        for(auto ent : group.second)
            groupTransforms.push_back(ent.getComponent<TransformComponent>().transform());
        instancedTransforms[group.first] = groupTransforms;
    }

    maybeUpdateDynamicShaders(scene);
    resizeOrClearResources();
    updateSSBOs(scene);

    glEnable(GL_DEPTH_TEST);
    if(config.enableCSM)
        CSMdepthPrePass(scene);
    pbrPass(scene);
    glDisable(GL_DEPTH_TEST);

    if(config.enableBloom)
        bloomComputePass();

    downsampleDepth();
    if(config.enableSSAO)
        ssaoPass(scene);

    if(config.enableVolumetric)
        volumetricPass(scene);

    upsamplePass();
    blurPass();
//    drawSceneDebug(scene);
    compositePass();
    drawScreenSpace(scene);

    outputFBO.bind();
    outputFBO.blitToFrontBuffer();
    drawImgui(scene);
}

void RenderPipeline::serialize()
{
    Serializer::serializeRenderConfig(config, "../Config/render_config.pc");
}

void RenderPipeline::initShaders()
{
    std::vector<ShaderFileDef> shaderSrcs = {
        {"../assets/shaders/basic_pbr.fs"},
        {"../assets/shaders/basic_pbr.vs"}
    };
    pbrShader = std::make_shared<Shader>("basic_pbr", shaderSrcs);
    AssetManager::addShader(pbrShader);

    shaderSrcs = {
        {"../assets/shaders/bloom_downsample.cmp"}
    };
    bloomDownsample = std::make_shared<Shader>("bloom_downsample", shaderSrcs);
    AssetManager::addShader(bloomDownsample);

    shaderSrcs = {
        {"../assets/shaders/bloom_upsample.cmp"}
    };
    bloomUpsample = std::make_shared<Shader>("bloom_upsample", shaderSrcs);
    AssetManager::addShader(bloomUpsample);

    shaderSrcs = {
        {"../assets/shaders/bilinear_downsample.cmp"}
    };
    bilinearDownsample = std::make_shared<Shader>("bilinear_downsample", shaderSrcs);
    AssetManager::addShader(bilinearDownsample);

    shaderSrcs = {
        {"../assets/shaders/tent_upsampleR11F_G11F_B10F.cmp"}
    };
    tentUpsampleDepth = std::make_shared<Shader>("tent_upsampleR11F_G11F_B10F", shaderSrcs);
    AssetManager::addShader(tentUpsampleDepth);

    shaderSrcs = {
        {"../assets/shaders/volumetric_upsample.cmp"}
    };
    upsampleVL = std::make_shared<Shader>("volumetric_upsample", shaderSrcs);
    AssetManager::addShader(upsampleVL);

    shaderSrcs = {
        {"../assets/shaders/perlin_texture.cmp"}
    };
    perlinNoiseGen = std::make_shared<Shader>("perlin_texture", shaderSrcs);
    AssetManager::addShader(perlinNoiseGen);

    shaderSrcs = {
        {
         "../assets/shaders/cascading_sm.gs", {"/*invocations*/"},
         {", invocations = "+std::to_string(config.shadowCascadeCount+1)}
        },
        {"../assets/shaders/cascading_sm.vs"},
        {"../assets/shaders/passthrough.fs"}
    };
    csmShader = std::make_shared<Shader>("cascading_sm", shaderSrcs);
    AssetManager::addShader(csmShader);

    shaderSrcs = {
        {"../assets/shaders/ssao.fs"},
        {"../assets/shaders/textured_passthrough.vs"}
    };
    ssaoShader = std::make_shared<Shader>("ssao", shaderSrcs);
    AssetManager::addShader(ssaoShader);

    shaderSrcs = {
        {"../assets/shaders/final_composite.fs"},
        {"../assets/shaders/textured_passthrough.vs"}
    };
    screenShader = std::make_shared<Shader>("final_composite", shaderSrcs);
    AssetManager::addShader(screenShader);

    shaderSrcs = {
        {"../assets/shaders/simple_blur.fs"},
        {"../assets/shaders/textured_passthrough.vs"}
    };
    simpleBlur = std::make_shared<Shader>("simple_blur", shaderSrcs);
    AssetManager::addShader(simpleBlur);

    shaderSrcs = {
        {"../assets/shaders/basic_pbr.vs"},
        {"../assets/shaders/solid_color.fs"}
    };
    solidShader = std::make_shared<Shader>("solid_color", shaderSrcs);
    AssetManager::addShader(solidShader);

    shaderSrcs = {
        {"../assets/shaders/textured_passthrough.vs"},
        {"../assets/shaders/volumetric_light.fs"}
    };
    vlShader = std::make_shared<Shader>("volumetric_light", shaderSrcs);
    AssetManager::addShader(vlShader);

}

void RenderPipeline::initFBOs()
{
    // Create main framebuffer
    FrameBufferAttachment colorAtt;
    colorAtt.type = GL_COLOR_ATTACHMENT0;
    colorAtt.format = GL_R11F_G11F_B10F;
    colorAtt.renderBuffer = false;

    FrameBufferAttachment bloomTresholdImage;
    bloomTresholdImage.type = GL_COLOR_ATTACHMENT1;
    bloomTresholdImage.format = GL_R11F_G11F_B10F;
    bloomTresholdImage.renderBuffer = false;

    FrameBufferAttachment depthAtt;
    depthAtt.type = GL_DEPTH_STENCIL_ATTACHMENT;
    depthAtt.format = GL_DEPTH24_STENCIL8;
    depthAtt.renderBuffer = false;

    hdrFBO = FrameBuffer(winSize.x, winSize.y, 1, {colorAtt, bloomTresholdImage}, depthAtt, 1);

    FrameBufferAttachment smDepthAtt;
    smDepthAtt.type = GL_DEPTH_ATTACHMENT;
    smDepthAtt.format = GL_DEPTH_COMPONENT32F;
    smDepthAtt.renderBuffer = false;
    csmFBO = FrameBuffer(config.csmResolution, config.csmResolution, config.shadowCascadeCount+1, {}, smDepthAtt, 1);

    FrameBufferAttachment ssaoAtt;
    ssaoAtt.type = GL_COLOR_ATTACHMENT0;
    ssaoAtt.format = GL_R32F;
    ssaoAtt.renderBuffer = false;

    if(config.ssaoHalfRes)
        ssaoFBO = FrameBuffer(winSize.x/2, winSize.y/2, 1, {ssaoAtt});
    else
        ssaoFBO = FrameBuffer(winSize.x, winSize.y, 1, {ssaoAtt});
    blurSsaoFBO = FrameBuffer(winSize.x, winSize.y, 1, {ssaoAtt});

    if(config.volumetricHalfRes)
        vlFBO = FrameBuffer(winSize.x/2, winSize.y/2, 1, {colorAtt});
    else
        vlFBO = FrameBuffer(winSize.x/2, winSize.y/2, 1, {colorAtt});
    blurVlFBO = FrameBuffer(winSize.x, winSize.y, 1, {colorAtt});

    downSampledDepth = std::make_shared<Texture>(winSize.x/2, winSize.y/2, 1, GL_R32F, GL_CLAMP_TO_EDGE);
    upSampledSSAO = std::make_shared<Texture>(winSize.x, winSize.y, 1, GL_R32F, GL_CLAMP_TO_EDGE);
    upSampledVL = std::make_shared<Texture>(winSize.x, winSize.y, 1, GL_R11F_G11F_B10F, GL_CLAMP_TO_EDGE);

    // Create output framebuffer
    colorAtt.renderBuffer = true;
    outputFBO = FrameBuffer(winSize.x, winSize.y, 1, {colorAtt});
}

void RenderPipeline::initSSAO()
{
    updateSSAOKernel();
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoNoise;
    for(uint i=0; i<16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    ssaoNoiseTex = std::make_shared<Texture>(4, 4, 1, GL_RGB16F, GL_REPEAT);
    ssaoNoiseTex->setData(ssaoNoise.data(), ssaoNoise.size()*sizeof(vec3));
}

void RenderPipeline::updateSSAOKernel()
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    ssaoKernel.clear();
    ssaoKernel.resize(config.ssaoKernelSize);
    for(int i=0; i<config.ssaoKernelSize; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i / (float)config.ssaoKernelSize);

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel[i] = sample;
    }
}

void RenderPipeline::initSSBOs()
{
    // Create lights SSBO
    lightsSSBO = StorageBuffer(MAX_LIGHTS*sizeof(GPU_PointLight));

    // Create light space matrix for DirLight
    csmSSBO = StorageBuffer(MAX_SHADOW_CASCADES*sizeof(mat4));

    // Create instanced transforms SSBO for pbr and csm
    instanceTransformsSSBO = StorageBuffer(MAX_INSTANCED*sizeof(mat4));

    // Create light space matrix for DirLight
    ssaoKernelSSBO = StorageBuffer(MAX_SSAO_KERNEL_SIZE*sizeof(vec3));
    ssaoKernelSSBO.setData(ssaoKernel.data(), config.ssaoKernelSize*sizeof(vec3));

    // Create perlin octaves ssbos
    perlinOctavesFogSSBO = StorageBuffer(MAX_NOISE_OCTAVES*sizeof(PerlinOctave));
    perlinOctavesFogSSBO.setData(config.perlinOctavesFog.data(), config.perlinOctavesFog.size()*sizeof(PerlinOctave));
    perlinOctavesTerrainSSBO = StorageBuffer(MAX_NOISE_OCTAVES*sizeof(PerlinOctave));
}

void RenderPipeline::maybeUpdateDynamicShaders(std::shared_ptr<Scene> scene)
{
    // Update runtime modified shaders
    if(config.shadowCascadeCount!=oldShadowCascadeCount)
    {
        ShaderFileDef def = {"../assets/shaders/cascading_sm.gs",
                             {"/*invocations*/"}, {", invocations = "+std::to_string(config.shadowCascadeCount)}};
        csmShader->updateRuntimeModifiedStrings(def);
        oldShadowCascadeCount = config.shadowCascadeCount;

//        LOG("Camera near:%f\n", scene->activeCamera()->getNearClip());
        updateCascadeRanges();
//        LOG("Camera far:%f\n", scene->activeCamera()->getFarClip());
    }
}

void RenderPipeline::updateSSBOs(std::shared_ptr<Scene> scene)
{
    // SSAO kernel update
    if(oldSsaoKernelSize != config.ssaoKernelSize)
    {
        updateSSAOKernel();
        ssaoKernelSSBO.setData(ssaoKernel.data(), ssaoKernel.size()*sizeof(vec3));
        oldSsaoKernelSize = config.ssaoKernelSize;
    }

    // Set point lights data
    std::vector<GPU_PointLight> lights;
    auto view = scene->entities().view<PointLightComponent>();
    for(auto ent : view)
        lights.push_back(view.get<PointLightComponent>(ent).light);

    enabledPointLightCount = (uint)lights.size();
    lightsSSBO.setData(lights.data(), sizeof(GPU_PointLight)*enabledPointLightCount);

    auto camera = scene->activeCamera();

    std::vector<glm::mat4> cascades;

    cascades.resize(config.shadowCascadeCount+1);
    for (int i=0; i<config.shadowCascadeCount+1; ++i)
    {
        float cascade = 0;
        float prevCascade = 0;
        if(config.shadowCascadeCount == 1)
        {
            cascade = camera->getFarClip();
            prevCascade = camera->getNearClip();
        }
        else
        {
            if (i==0)
            {
                cascade = cascadeRanges[i];
                prevCascade = camera->getNearClip();
            }
            else if (i<config.shadowCascadeCount)
            {
                cascade = cascadeRanges[i];
                prevCascade = camera->getNearClip();
            }
            else
            {
                cascade = camera->getFarClip();
                prevCascade = camera->getNearClip();
            }
        }
        cascades[i] = calcDirLightViewProjMatrix(camera, scene->directionalLight, prevCascade,
                                                 cascade, config.cascadeZextra);
    }
    csmSSBO.setData(cascades.data(), sizeof(mat4)*cascades.size());
}

void RenderPipeline::resizeBloomBuffers()
{
    float minDim = (float)glm::min(winSize.x, winSize.y);
    float bSamples = (config.bloomRadius - 8.0f) + log(minDim) / log(2.f);
    bloomSamples = (int)bSamples < MAX_BLOOM_SAMPLES ? (int)bSamples : MAX_BLOOM_SAMPLES;

    uint prevSize = (uint)bloomDownSampleTextures.size();

//    LOG("Resizing bloom buffers:\n    prevSize: %d, newSize: %d\n", prevSize, bloomSamples);

    bloomDownSampleTextures.resize(bloomSamples);
    bloomUpSampleTextures.resize(bloomSamples);

    uvec2 uWinSize = (uvec2) winSize;
    // resize textures that are left after the resize
    uint size = prevSize>bloomSamples ? bloomSamples : prevSize;
    for(uint i=0; i<size; i++)
    {
        uint width  = (uint)(uWinSize.x/(pow(2.f,i+1)));
        uint height = (uint)(uWinSize.y/(pow(2.f,i+1)));
        bloomDownSampleTextures[i]->resize({width, height, 1});
//        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=0; i<size; i++)
    {
        uint width  = (uint)(uWinSize.x/(pow(2.f,i)));
        uint height = (uint)(uWinSize.y/(pow(2.f,i)));
        bloomUpSampleTextures[i]->resize({width, height, 1});
//        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }

    if(prevSize>=bloomSamples)
        return;

    // add new textures if the new size is bigger
    for(uint i=prevSize; i<bloomSamples; i++)
    {
        uint width  = (uint)(uWinSize.x/pow(2.f,i+1));
        uint height = (uint)(uWinSize.y/pow(2.f,i+1));
        bloomDownSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, GL_CLAMP_TO_EDGE);
//        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=prevSize; i<bloomSamples; i++)
    {
        uint width  = (uint)(uWinSize.x/pow(2.f,i));
        uint height = (uint)(uWinSize.y/pow(2.f,i));
        bloomUpSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, GL_CLAMP_TO_EDGE);
//        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }
}

void RenderPipeline::updateCascadeRanges()
{
    cascadeRanges.resize(config.shadowCascadeCount);
    for (int i=0; i<config.shadowCascadeCount; ++i)
    {
        cascadeRanges[i] = (uint)glm::pow(3, i+1)+config.firstCascadeOffset;
//        LOG("Cascade %d: %f\n", i, cascadeRanges[i]);
    }
}

void RenderPipeline::CSMdepthPrePass(std::shared_ptr<Scene> scene)
{
    PROFILE_GPU("CSMPass", syncGPU);
    UNUSED(scene);
    // Draw cascading shadow maps to depth
    csmFBO.bind();
    csmSSBO.bind(1);
    csmShader->bind();
    glDisable(GL_BLEND);
    glCullFace(GL_FRONT);

    // Render single entities
    csmShader->setUniform("u_DrawInstanced", BufferElement::Int, 0);
    auto view = scene->entities().view<TransformComponent, MeshComponent, RenderSpecComponent>(entt::exclude<InstancedDrawComponent>);
    for(auto& ent : view)
    {
        // draw here
        auto transform = view.get<TransformComponent>(ent).transform();
        csmShader->setUniform("u_Model", BufferElement::Mat4, transform);

        auto model = view.get<MeshComponent>(ent).model;
        for(auto mesh : model->meshes())
        {
            auto vao = mesh->vao();
            vao->bind();
            glDrawElements(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, nullptr);
            vao->unbind();
        }
    }

    // Draw instanced
    csmShader->setUniform("u_DrawInstanced", BufferElement::Int, 1);
    for(auto& group : instancedGroups)
    {
        ASSERT(group.second.size()>0);
        auto grpTransforms = instancedTransforms[group.first];
        instanceTransformsSSBO.setData(grpTransforms.data(), grpTransforms.size()*sizeof(mat4));
        instanceTransformsSSBO.bind(2);
        auto model = group.second[0].getComponent<MeshComponent>().model;
        for(auto mesh : model->meshes())
        {
            auto vao = mesh->vao();
            vao->bind();
            glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, 0, group.second.size());
            vao->unbind();
        }
        instanceTransformsSSBO.unbind();
    }

    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    csmShader->unbind();
    csmFBO.unbind();
}

void RenderPipeline::pbrPass(std::shared_ptr<Scene> scene)
{
    PROFILE_GPU("PBRPass", syncGPU);
    // DRAW pbr objects
    hdrFBO.bind();
    auto sceneCamera = scene->activeCamera();
    pbrShader->bind();

    // Pointlights
    lightsSSBO.bind(0);
    pbrShader->setUniform("u_PointLightCount", BufferElement::Uint, enabledPointLightCount);

    // Set camera data
    pbrShader->setUniform("u_View", BufferElement::Mat4, sceneCamera->getViewMatrix());
    pbrShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    pbrShader->setUniform("u_CameraPosition", BufferElement::Float3, sceneCamera->getPos());

    // Set directional light data
    pbrShader->setUniform("u_AmbientIntensity", BufferElement::Float, scene->directionalLight.ambientIntensity);
    pbrShader->setUniform("u_DirLightDirection", BufferElement::Float3, scene->directionalLight.direction);
    pbrShader->setUniform("u_DirLightCol", BufferElement::Float3, scene->directionalLight.color);
    pbrShader->setUniform("u_DirLightIntensity", BufferElement::Float, scene->directionalLight.intensity);

    // CSM data
    csmSSBO.bind(1);
    pbrShader->setUniform("u_farPlane", BufferElement::Float, sceneCamera->getFarClip());
    pbrShader->setUniform("u_cascadeCount", BufferElement::Int, config.shadowCascadeCount+1);
    pbrShader->setUniformArray("u_cascadePlaneDistances", BufferElement::Float, cascadeRanges.data(), (uint)cascadeRanges.size());
    csmFBO.getDepthTex()->bind(3);

    // this below should probably go to separate pass
    pbrShader->setUniform("u_bloomTreshold", BufferElement::Float, config.bloomTreshold);
    pbrShader->setUniform("u_exposure", BufferElement::Float, config.exposure);


    // Render single entities
    pbrShader->setUniform("u_DrawInstanced", BufferElement::Int, 0);
    auto view = scene->entities().view<TransformComponent, MeshComponent, RenderSpecComponent>(entt::exclude<InstancedDrawComponent>);
    for(auto& ent : view)
    {
        auto renderSpec = view.get<RenderSpecComponent>(ent);
        auto transform = view.get<TransformComponent>(ent).transform();
        auto model = view.get<MeshComponent>(ent).model;
        pbrShader->setUniform("u_Model", BufferElement::Mat4, transform);

        for(auto mesh : model->meshes())
        {
            auto vao = mesh->vao();

            //draw with basic_pbr
            pbrShader->setUniform("u_Metallic", BufferElement::Float, mesh->material.metallic);
            pbrShader->setUniform("u_Roughness", BufferElement::Float, mesh->material.roughness);

            //NOTE: We might add this as a boolean check but you shouldn't set override color to
            //      invisible, instead you should set renderable to false
            if(renderSpec.color.a > 0)
                pbrShader->setUniform("u_Color", BufferElement::Float4, renderSpec.color);
            else
                pbrShader->setUniform("u_Color", BufferElement::Float4, mesh->material.color);
            vao->bind();
            glDrawElements(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, nullptr);
            vao->unbind();
        }
    }

    // Draw instanced
    pbrShader->setUniform("u_DrawInstanced", BufferElement::Int, 1);
    for(auto& group : instancedGroups)
    {
        ASSERT(group.second.size()>0);
        auto grpTransforms = instancedTransforms[group.first];
        instanceTransformsSSBO.setData(grpTransforms.data(), grpTransforms.size()*sizeof(mat4));
        instanceTransformsSSBO.bind(2);
        auto model = group.second[0].getComponent<MeshComponent>().model;
        for(auto mesh : model->meshes())
        {
            auto vao = mesh->vao();

            //draw with basic_pbr
            pbrShader->setUniform("u_Metallic", BufferElement::Float, mesh->material.metallic);
            pbrShader->setUniform("u_Roughness", BufferElement::Float, mesh->material.roughness);
            pbrShader->setUniform("u_Color", BufferElement::Float4, mesh->material.color);
            vao->bind();
            glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, 0, group.second.size());
            vao->unbind();
        }
        instanceTransformsSSBO.unbind();
    }


    lightsSSBO.unbind();
    hdrFBO.unbind();
    csmSSBO.unbind();

}

void RenderPipeline::bloomComputePass()
{
    PROFILE_GPU("Bloom compute", syncGPU);
    // Bloom compute
    for(uint i=0; i<bloomSamples; i++)
    {
        //Downsample and blur
        bloomDownsample->bind();

        vec2 size;
        if(i==0)
        {
            hdrFBO.getTexture(1)->bind(1);
            size = hdrFBO.getTexture(1)->getDimensions();
        }
        else
        {
            bloomDownSampleTextures[i-1]->bind(1);
            size = bloomDownSampleTextures[i-1]->getDimensions();
        }

        bloomDownsample->bindImage(bloomDownSampleTextures[i], 0, GL_WRITE_ONLY, false);
        bloomDownsample->dispatch((uint)ceil(size.x/16), (uint)ceil(size.y/16), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        bloomDownsample->unbind();
    }
    for(int i=bloomSamples-1; i>=0; i--)
    {
        // Upsample
        bloomUpsample->bind();
        vec2 size;
        if(i==0)
        {
            hdrFBO.getTexture(1)->bind(1);
            size = hdrFBO.getTexture(1)->getDimensions();
        }
        else
        {
            bloomDownSampleTextures[i-1]->bind(2);
            size = bloomDownSampleTextures[i-1]->getDimensions();
        }

        if((uint)i==bloomSamples-1)
            bloomDownSampleTextures[i]->bind(1);
        else
            bloomUpSampleTextures[i+1]->bind(1);

        bloomUpsample->bindImage(bloomUpSampleTextures[i], 0,  GL_WRITE_ONLY, false);
        bloomUpsample->dispatch((uint)ceil(size.x/32), (uint)ceil(size.y/32), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        bloomUpsample->unbind();
    }
}

void RenderPipeline::downsampleDepth()
{
    PROFILE_GPU("DownsampleDepth", syncGPU);
    if(config.ssaoHalfRes || config.volumetricHalfRes)
    {
        glBindTexture(GL_TEXTURE_2D, NULL);
        bilinearDownsample->bind();
        hdrFBO.getDepthTex()->bind(1);
        bilinearDownsample->bindImage(downSampledDepth, 0, GL_WRITE_ONLY, false);
        vec2 size = hdrFBO.getDepthTex()->getDimensions();
        bilinearDownsample->dispatch((uint)ceil(size.x/16), (uint)ceil(size.y/16), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        bilinearDownsample->unbind();
    }
}

void RenderPipeline::volumetricPass(std::shared_ptr<Scene> scene)
{
    PROFILE_GPU("VolumetricPass", syncGPU);
    auto sceneCamera = scene->activeCamera();
    vlShader->bind();
    vlFBO.bind();

    //Volumetric settings
    vlShader->setUniform("u_Samples", BufferElement::Int, config.volumetricSamples);
    vlShader->setUniform("u_Gfactor", BufferElement::Float, config.g_factor);
    vlShader->setUniform("u_FogStrength", BufferElement::Float, config.fog_strength);
    vlShader->setUniform("u_FogY", BufferElement::Float, config.fog_y);
    vlShader->setUniform("u_LightShaftIntensity", BufferElement::Float, config.lightShaftIntensity);
    vlShader->setUniform("u_timeAccum", BufferElement::Float, (float)App::getTime());
    perlinTextureFog->bind(2);

    //Camera
    vlShader->setUniform("u_View", BufferElement::Mat4, sceneCamera->getViewMatrix());
    vlShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    vlShader->setUniform("u_CameraPosition", BufferElement::Float3, sceneCamera->getPos());

    // Lightning
    vlShader->setUniform("u_DirLightDirection", BufferElement::Float3, scene->directionalLight.direction);
    vlShader->setUniform("u_DirLightCol", BufferElement::Float3, scene->directionalLight.color);
    vlShader->setUniform("u_DirLightIntensity", BufferElement::Float, scene->directionalLight.intensity);
    vlShader->setUniform("u_AmbientIntensity", BufferElement::Float, scene->directionalLight.ambientIntensity);
    vlShader->setUniform("u_PointLightCount", BufferElement::Uint, enabledPointLightCount);
    lightsSSBO.bind(0);

    // CSM data
    csmSSBO.bind(1);
    vlShader->setUniform("u_nearPlane", BufferElement::Float, sceneCamera->getNearClip());
    vlShader->setUniform("u_farPlane", BufferElement::Float, sceneCamera->getFarClip());
    vlShader->setUniform("u_cascadeCount", BufferElement::Int, config.shadowCascadeCount+1);
    vlShader->setUniformArray("u_cascadePlaneDistances", BufferElement::Float, cascadeRanges.data(), (uint)cascadeRanges.size());
    csmFBO.getDepthTex()->bind(0);


    if(config.volumetricHalfRes)
        downSampledDepth->bind(1);
    else
        hdrFBO.getDepthTex()->bind(1);


    screenQuad.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();
    vlFBO.unbind();
    vlShader->unbind();
}

void RenderPipeline::ssaoPass(std::shared_ptr<Scene> scene)
{
    PROFILE_GPU("SSAOPass", syncGPU);
    auto sceneCamera = scene->activeCamera();

    ssaoFBO.bind();
    ssaoShader->bind();
    ssaoShader->setUniform("kernelSize", BufferElement::Int, config.ssaoKernelSize);
    ssaoShader->setUniform("radius", BufferElement::Float, config.ssaoRadius);
    ssaoShader->setUniform("bias", BufferElement::Float, config.ssaoBias);
    ssaoShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());

    if(config.ssaoHalfRes)
        downSampledDepth->bind(0);
    else
        hdrFBO.getDepthTex()->bind(0);

    ssaoNoiseTex->bind(1);
    ssaoKernelSSBO.bind(0);

    screenQuad.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();

    ssaoShader->unbind();
    ssaoKernelSSBO.unbind();
    ssaoFBO.unbind();
}

void RenderPipeline::upsamplePass()
{
    PROFILE_GPU("UpsamplePass", syncGPU);
    if(config.ssaoHalfRes && config.enableSSAO)
    {
        tentUpsampleDepth->bind();
        tentUpsampleDepth->bindImage(upSampledSSAO, 0, GL_WRITE_ONLY, false);
        ssaoFBO.getTexture(0)->bind(1);

        vec2 size = upSampledSSAO->getDimensions();
        tentUpsampleDepth->dispatch((uint)ceil(size.x/32), (uint)ceil(size.y/32), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        tentUpsampleDepth->unbind();
    }

    if(config.volumetricHalfRes && config.enableVolumetric)
    {
        upsampleVL->bind();
        upsampleVL->bindImage(upSampledVL, 0, GL_WRITE_ONLY, false);
        downSampledDepth->bind(1);
        hdrFBO.getDepthTex()->bind(2);
        vlFBO.getTexture(0)->bind(3);

        vec2 size = upSampledVL->getDimensions();
        upsampleVL->dispatch((uint)ceil(size.x/32), (uint)ceil(size.y/32), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        upsampleVL->unbind();
    }
}

void RenderPipeline::blurPass()
{
    PROFILE_GPU("BlurPass", syncGPU);
    simpleBlur->bind();

    if(config.enableSSAO)
    {
        blurSsaoFBO.bind();

        if(config.ssaoHalfRes)
            upSampledSSAO->bind(0);
        else
            ssaoFBO.getTexture(0)->bind(0);

        simpleBlur->setUniform("kernelSize", BufferElement::Int, config.ssaoBlurSize);

        screenQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        screenQuad.unbind();

        blurSsaoFBO.unbind();
    }

    if(config.enableVolumetric)
    {
        blurVlFBO.bind();
        if(config.volumetricHalfRes)
            upSampledVL->bind(0);
        else
            vlFBO.getTexture(0)->bind(0);

        simpleBlur->setUniform("kernelSize", BufferElement::Int, config.volumetricBlurSize);

        screenQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        screenQuad.unbind();

        blurVlFBO.unbind();
    }

    simpleBlur->unbind();
}

void RenderPipeline::compositePass()
{
    PROFILE_GPU("CompositePass", syncGPU);
    // Draw final ouput image
    outputFBO.bind();
    screenShader->bind();
    screenShader->setUniform("u_bloomIntensity", BufferElement::Float, config.bloomIntensity);
    hdrFBO.getTexture(0)->bind(0);
    blurSsaoFBO.getTexture(0)->bind(1);
    if(bloomUpSampleTextures.size()>0)
        bloomUpSampleTextures[0]->bind(2);
    blurVlFBO.getTexture(0)->bind(3);

    screenQuad.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();

    screenShader->unbind();
    outputFBO.unbind();
}

void RenderPipeline::resizeOrClearResources()
{
    // Resize based on config changes
    if(config.ssaoHalfRes != oldSsaoHalfRes)
    {
        if(config.ssaoHalfRes)
            ssaoFBO.resize(winSize.x/2, winSize.y/2, 1);
        else
            ssaoFBO.resize(winSize.x, winSize.y, 1);
        oldSsaoHalfRes = config.ssaoHalfRes;
    }

    if(config.volumetricHalfRes != oldVlHalfRes)
    {
        if(config.volumetricHalfRes)
            vlFBO.resize(winSize.x/2, winSize.y/2, 1);
        else
            vlFBO.resize(winSize.x, winSize.y, 1);
        oldVlHalfRes = config.volumetricHalfRes;
    }

    if(config.csmResolution != oldCsmResolusion)
    {
        csmFBO.resize(config.csmResolution, config.csmResolution, config.shadowCascadeCount+1);
        oldCsmResolusion = config.csmResolution;
    }

    if(!config.enableBloom && bloomUpSampleTextures.size()>0)
        bloomUpSampleTextures[0]->clear({0,0,0});
    if(!config.enableSSAO)
        blurSsaoFBO.getTexture(0)->clear({1,1,1});
    // Resize FBOs and textures
    if(config.bloomRadius != oldBloomRadius)
    {
        resizeBloomBuffers();
        oldBloomRadius = config.bloomRadius;
    }

    // resize based on resolution changes
    if(winSize != oldWinSize)
    {
        hdrFBO.resize(winSize.x, winSize.y, 1);
        outputFBO.resize(winSize.x, winSize.y, 1);

        if(config.ssaoHalfRes)
            ssaoFBO.resize(winSize.x/2, winSize.y/2, 1);
        else
            ssaoFBO.resize(winSize.x, winSize.y, 1);

        if(config.volumetricHalfRes)
            vlFBO.resize(winSize.x/2, winSize.y/2, 1);
        else
            vlFBO.resize(winSize.x, winSize.y, 1);

        blurSsaoFBO.resize(winSize.x, winSize.y, 1);
        blurVlFBO.resize(winSize.x, winSize.y, 1);
        downSampledDepth->resize({ceil(winSize.x/2.f), ceil(winSize.y/2.f), 1});
        upSampledSSAO->resize({winSize.x, winSize.y, 1});
        upSampledVL->resize({winSize.x, winSize.y, 1});
        resizeBloomBuffers();
    }
    else
    {
        // Clear FBOs and textures if we didnt resize the window
        hdrFBO.clearColorAttachment(0, {0.302f, 0.345f, 0.388f});
        hdrFBO.clearColorAttachment(1, {0, 0, 0});
        hdrFBO.clearDepthAttachment();
        csmFBO.clearDepthAttachment();
        outputFBO.clear({0.5, 0.5, 0.5});
        ssaoFBO.clear({0.5, 0.5, 0.5});
        blurSsaoFBO.clear({1, 1, 1});
        blurVlFBO.clear({0,0,0});
        vlFBO.clear({0,0,0});

//        for(uint i=1; i<bloomSamples+1; i++)
//        {
//            bloomDownSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
//            bloomUpSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
//        }
    }
}

void RenderPipeline::generateNoise()
{
    perlinNoiseGen->bind();
    perlinOctavesFogSSBO.bind(0);
    perlinNoiseGen->setUniform("u_octaveCount", BufferElement::Int, (int)config.perlinOctavesFog.size());
    perlinNoiseGen->bindImage(perlinTextureFog, 0, GL_WRITE_ONLY, false);

    vec3 size = perlinTextureFog->getDimensions();
    perlinNoiseGen->dispatch((uint)ceil(size.x/16), (uint)ceil(size.y/16), (uint)ceil(size.z/4));

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    perlinNoiseGen->unbind();
}

void RenderPipeline::drawSceneDebug(std::shared_ptr<Scene> scene)
{
    auto sceneCamera = scene->activeCamera();
    hdrFBO.bindColorAttachment(0);
    // Draw debug graphics
    BatchRenderer::begin(sceneCamera);
    scene->onDebugDraw();
    BatchRenderer::end();
    hdrFBO.unbind();
}

void RenderPipeline::drawScreenSpace(std::shared_ptr<Scene> scene)
{
    auto sceneCamera = scene->activeCamera();
    outputFBO.bind();
    if(App::getKeyOnce(GLFW_KEY_KP_ADD))
        debugView++;
    if(App::getKeyOnce(GLFW_KEY_KP_SUBTRACT))
        debugView--;
    if(App::getKeyOnce(GLFW_KEY_KP_0))
        debugView = 0;
    glm::clamp(debugView, 0, 5);
    BatchRenderer::begin(sceneCamera);
    switch(debugView)
    {
    case 0: break;
    case 1:
    {
        perlinTextureFog->selectLayerForNextDraw(0);
        BatchRenderer::drawQuad({0,0}, perlinTextureFog->getDimensions(), perlinTextureFog); break;
    }
    case 2: BatchRenderer::drawQuad({0,0}, winSize, blurVlFBO.getTexture(0)); break;
    case 3: BatchRenderer::drawQuad({0,0}, winSize, blurSsaoFBO.getTexture(0)); break;
    case 4: BatchRenderer::drawQuad({0,0}, winSize, bloomUpSampleTextures[0]); break;
    case 5: BatchRenderer::drawQuad({0,0}, winSize, csmFBO.getDepthTex()); break;
    default:
        break;
    }

    BatchRenderer::end();
    outputFBO.unbind();
}

void RenderPipeline::guiNoiseSettings()
{
    if(ImGui::Begin("Noise settings", &showNoiseSettings))
    {
        const char* items[] = { "Fog", "Terrain" };
        static int item_current = 0;
        ImGui::Combo("##combo", &item_current, items, IM_ARRAYSIZE(items));
        std::vector<PerlinOctave>* octaves;
        std::vector<PerlinOctave> dummy;
        bool changed = false;

        std::shared_ptr<Texture> tex = nullptr;
        switch(item_current)
        {
        case 0: octaves = &config.perlinOctavesFog; tex = perlinTextureFog; break;
        case 1: octaves = &config.perlinOctavesTerrain; tex = perlinTextureTerrain; break;
        default: octaves = &dummy; break;
        }


        static int selectedLayer = 0;
        TWEAK_INT("Layer", selectedLayer, 1, 0, tex->getDimensions().z);

        auto winPos = ImGui::GetWindowPos();
        auto winWidth = ImGui::GetWindowWidth();

        BatchRenderer::begin(nullptr);
        perlinTextureFog->selectLayerForNextDraw(selectedLayer);
        BatchRenderer::drawQuad({winPos.x + winWidth,winPos.y}, {300, 300}, perlinTextureFog);
        BatchRenderer::end();


        if (ImGui::Button("Add octave")) { octaves->push_back({}); changed = true;}
        for(size_t i=0; i<octaves->size(); i++)
        {
            auto name = "Octave "+std::to_string(i);
            if (ImGui::TreeNode(name.c_str()))
            {
                if (ImGui::Button("Remove")) { octaves->erase(octaves->begin()+i); i--; changed = true;}
                auto octave = (octaves->data()+i);
                changed |= TWEAK_VEC3("Frequencies", octave->frequency, 0.1f);
                changed |= TWEAK_VEC3("Offsets", octave->offset, 0.1f);
                changed |= TWEAK_FLOAT("Amplitude", octave->amplitude, 0.1f);
                ImGui::TreePop();
            }
        }
        if(changed)
        {
            switch(item_current)
            {
            case 0: perlinOctavesFogSSBO.setData(config.perlinOctavesFog.data(), config.perlinOctavesFog.size()*sizeof(PerlinOctave)); break;
            case 1: perlinOctavesTerrainSSBO.setData(config.perlinOctavesTerrain.data(), config.perlinOctavesTerrain.size()*sizeof(PerlinOctave)); break;
            default: break;
            }
            generateNoise();
        }
    }
    ImGui::End();
}

void RenderPipeline::drawImgui(std::shared_ptr<Scene> scene)
{
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
    if(ImGui::Begin("Render settings", &showRenderSettings))
    {
        // CSM
        if (ImGui::CollapsingHeader("CSM"))
        {
            TWEAK_BOOL("enableCSM", config.enableCSM);
            TWEAK_INT("shadowCascadeCount", config.shadowCascadeCount, 1, 1, 10);
            TWEAK_FLOAT("firstCascadeOffset", config.firstCascadeOffset, 0.01f);
            TWEAK_FLOAT("cascadeZextra", config.cascadeZextra, 0.01f);
            TWEAK_INT("csmResolution", config.csmResolution, 256, 256, 4096)
        }

        // BLOOM
        if (ImGui::CollapsingHeader("Bloom"))
        {
            TWEAK_BOOL("enableBloom", config.enableBloom);
            TWEAK_FLOAT("bloomRadius", config.bloomRadius, 0.01f, 0, 8);
            TWEAK_FLOAT("bloomIntensity", config.bloomIntensity, 0.01f);
            TWEAK_FLOAT("bloomTreshold", config.bloomTreshold, 0.01f);
            TWEAK_FLOAT("exposure", config.exposure, 0.01f);
        }

        // SSAO
        if (ImGui::CollapsingHeader("SSAO"))
        {
            TWEAK_BOOL("enableSSAO", config.enableSSAO);
            TWEAK_BOOL("halfRes", config.ssaoHalfRes);
            TWEAK_INT("blurSize", config.ssaoBlurSize, 2, 2, 20);
            TWEAK_INT("ssaoKernelSize", config.ssaoKernelSize, 2, 2, 256);
            TWEAK_FLOAT("ssaoRadius", config.ssaoRadius, 0.01f);
            TWEAK_FLOAT("ssaoBias", config.ssaoBias, 0.01f);
        }

        //Volumetric
        if (ImGui::CollapsingHeader("Volumetric"))
        {
            TWEAK_BOOL("enableVolumetric", config.enableVolumetric);
            TWEAK_BOOL("halfRes", config.volumetricHalfRes);
            TWEAK_INT("samples", config.volumetricSamples, 1, 1, 200);
            TWEAK_FLOAT("gFactor", config.g_factor, 0.001, -10, 10);
            TWEAK_FLOAT("fogY", config.fog_y, 0.01f);
            TWEAK_FLOAT("fogStrength", config.fog_strength, 0.01f);
            TWEAK_FLOAT("lightShaftIntensity", config.lightShaftIntensity, 0.01f)
            TWEAK_INT("blurSize", config.volumetricBlurSize, 2, 2, 20);
        }
    }
    ImGui::End();

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (config.renderStatsCorner != -1)
        {
            const float PAD = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (config.renderStatsCorner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (config.renderStatsCorner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (config.renderStatsCorner & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (config.renderStatsCorner & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Example: Simple overlay", &showRenderStats, window_flags))
        {
            ImGui::Text("Render profile:");
            ImGui::Separator();
            ImGui::Checkbox("Synchronize GPU", &syncGPU);
            auto storage = InstrumentationStorage::getInstance().storage;
            ProfileResult gpuRes;
            ProfileResult cpuRes;
            for(auto& [key, result] : storage)
            {
                if(result.name == "GPU")
                    gpuRes = result;
                else if(result.name == "CPU")
                    cpuRes = result;
                else
                    ImGui::Text("%s: %.3f ms", result.name.c_str(), result.ms);

            }
            ImGui::Separator();
            ImGui::Text("%s: %.3f ms", gpuRes.name.c_str(), gpuRes.ms);
            ImGui::Text("%s: %.3f ms", cpuRes.name.c_str(), cpuRes.ms);
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom",       NULL, config.renderStatsCorner == -1)) config.renderStatsCorner = -1;
                if (ImGui::MenuItem("Top-left",     NULL, config.renderStatsCorner == 0)) config.renderStatsCorner = 0;
                if (ImGui::MenuItem("Top-right",    NULL, config.renderStatsCorner == 1)) config.renderStatsCorner = 1;
                if (ImGui::MenuItem("Bottom-left",  NULL, config.renderStatsCorner == 2)) config.renderStatsCorner = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, config.renderStatsCorner == 3)) config.renderStatsCorner = 3;
                if (showRenderStats && ImGui::MenuItem("Close")) showRenderStats = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
    //Set all reading to zero
    for(auto& [key, result] : InstrumentationStorage::getInstance().storage)
        result.ms = 0;
    scene->sceneSettingsRender();
    guiNoiseSettings();
}
