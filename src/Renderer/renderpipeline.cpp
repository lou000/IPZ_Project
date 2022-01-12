#include "renderpipeline.h"
#include"../Core/yamlserialization.h"
#include "../Core/gui.h"
#include "../Core/application.h"
#include "../Core/entity.h"
#include "../Core/components.h"
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


    // TODO: move everything below this to primitive generation
    //---------------------------------------------------------------//

    // Create white texture for non-textured drawing
    whiteTexture = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTexture->setTextureData(&whiteData, sizeof(uint));

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

    if(config.enableSSAO)
    {
        ssaoPass(scene);
        ssaoBlur();
    }
//    drawSceneDebug(scene);
    compositePass();
    drawScreenSpace(scene);

    outputFBO.bind();
    outputFBO.blitToFrontBuffer();
    drawImgui();
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
    downsampleAndBlur = std::make_shared<Shader>("bloom_downsample", shaderSrcs);
    AssetManager::addShader(downsampleAndBlur);

    shaderSrcs = {
        {"../assets/shaders/bloom_upsample.cmp"}
    };
    tentUpsampleAndAdd = std::make_shared<Shader>("bloom_upsample", shaderSrcs);
    AssetManager::addShader(tentUpsampleAndAdd);

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

    hdrFBO = FrameBuffer(winSize.x, winSize.y, 1, {colorAtt, bloomTresholdImage}, depthAtt, 0);

    FrameBufferAttachment smDepthAtt;
    smDepthAtt.type = GL_DEPTH_ATTACHMENT;
    smDepthAtt.format = GL_DEPTH_COMPONENT32F;
    smDepthAtt.renderBuffer = false;
    csmFBO = FrameBuffer(config.csmResolution, config.csmResolution, config.shadowCascadeCount+1, {}, smDepthAtt, 0);

    FrameBufferAttachment ssaoAtt;
    ssaoAtt.type = GL_COLOR_ATTACHMENT0;
    ssaoAtt.format = GL_R16F;
    ssaoAtt.renderBuffer = false;

    ssaoFBO = FrameBuffer(winSize.x, winSize.y, 1, {ssaoAtt});
    blurFBO = FrameBuffer(winSize.x, winSize.y, 1, {ssaoAtt});

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
    ssaoNoiseTex->setTextureData(ssaoNoise.data(), ssaoNoise.size()*sizeof(vec3));
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
    lightsSSBO = StorageBuffer(MAX_LIGHTS*sizeof(GPU_PointLight), 0);

    // Create light space matrix for DirLight
    csmSSBO = StorageBuffer(MAX_SHADOW_CASCADES*sizeof(mat4), 1);

    // Create light space matrix for DirLight
    ssaoKernelSSBO = StorageBuffer(MAX_SSAO_KERNEL_SIZE*sizeof(vec3), 4);
    ssaoKernelSSBO.setData(ssaoKernel.data(), config.ssaoKernelSize*sizeof(vec3));
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

        LOG("Camera near:%f\n", scene->activeCamera()->getNearClip());
        updateCascadeRanges();
        LOG("Camera far:%f\n", scene->activeCamera()->getFarClip());
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
    auto&& storage = scene->entities().storage<PointLightComponent>();

    enabledPointLightCount = (uint)storage.size();
    lightsSSBO.setData(storage.raw(), sizeof(GPU_PointLight)*enabledPointLightCount);

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

    LOG("Resizing bloom buffers:\n    prevSize: %d, newSize: %d\n", prevSize, bloomSamples);

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
        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=0; i<size; i++)
    {
        uint width  = (uint)(uWinSize.x/(pow(2.f,i)));
        uint height = (uint)(uWinSize.y/(pow(2.f,i)));
        bloomUpSampleTextures[i]->resize({width, height, 1});
        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }

    if(prevSize>=bloomSamples)
        return;

    // add new textures if the new size is bigger
    for(uint i=prevSize; i<bloomSamples; i++)
    {
        uint width  = (uint)(uWinSize.x/pow(2.f,i+1));
        uint height = (uint)(uWinSize.y/pow(2.f,i+1));
        bloomDownSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, GL_CLAMP_TO_EDGE);
        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=prevSize; i<bloomSamples; i++)
    {
        uint width  = (uint)(uWinSize.x/pow(2.f,i));
        uint height = (uint)(uWinSize.y/pow(2.f,i));
        bloomUpSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, GL_CLAMP_TO_EDGE);
        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }
}

void RenderPipeline::updateCascadeRanges()
{
    cascadeRanges.resize(config.shadowCascadeCount);
    for (int i=0; i<config.shadowCascadeCount; ++i)
    {
        cascadeRanges[i] = (uint)glm::pow(3, i+1)+config.firstCascadeOffset;
        LOG("Cascade %d: %f\n", i, cascadeRanges[i]);
    }
}

void RenderPipeline::CSMdepthPrePass(std::shared_ptr<Scene> scene)
{
    UNUSED(scene);
    // Draw cascading shadow maps to depth
    csmFBO.bind();
    csmSSBO.bind();
    csmShader->bind();
    glDisable(GL_BLEND);
    glCullFace(GL_FRONT);
    auto group = scene->entities().group<TransformComponent, MeshComponent, RenderSpecComponent>();
    for(auto& ent : group)
    {
        // draw here
        auto transform = group.get<TransformComponent>(ent).transform();
        csmShader->setUniform("u_Model", BufferElement::Mat4, transform);

        auto model = group.get<MeshComponent>(ent).model;
        for(auto mesh : model->meshes())
        {
            auto vao = mesh->vao();
            vao->bind();
            glDrawElements(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, nullptr);
            vao->unbind();
        }
    }
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    csmShader->unbind();
    csmFBO.unbind();
}

void RenderPipeline::pbrPass(std::shared_ptr<Scene> scene)
{
    // DRAW pbr objects
    hdrFBO.bind();
    auto sceneCamera = scene->activeCamera();
    pbrShader->bind();
    lightsSSBO.bind();

    // Set camera data
    pbrShader->setUniform("u_View", BufferElement::Mat4, sceneCamera->getViewMatrix());
    pbrShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    pbrShader->setUniform("u_CameraPosition", BufferElement::Float3, sceneCamera->getPos());

    // Set directional light data
    pbrShader->setUniform("u_DirLightDirection", BufferElement::Float3, scene->directionalLight.direction);
    pbrShader->setUniform("u_DirLightCol", BufferElement::Float3, scene->directionalLight.color);
    pbrShader->setUniform("u_DirLightIntensity", BufferElement::Float, scene->directionalLight.intensity);
    pbrShader->setUniform("u_PointLightCount", BufferElement::Uint, enabledPointLightCount);

    // CSM data
    pbrShader->setUniform("u_farPlane", BufferElement::Float, sceneCamera->getFarClip());
    pbrShader->setUniform("u_cascadeCount", BufferElement::Int, config.shadowCascadeCount+1);
    pbrShader->setUniformArray("u_cascadePlaneDistances", BufferElement::Float, cascadeRanges.data(), (uint)cascadeRanges.size());
    csmFBO.getDepthTex()->bind(3);

    // this below should probably go to separate draw call
    pbrShader->setUniform("u_bloomTreshold", BufferElement::Float, config.bloomTreshold);
    pbrShader->setUniform("u_exposure", BufferElement::Float, config.exposure);


    auto group = scene->entities().group<TransformComponent, MeshComponent, RenderSpecComponent>();
    for(auto& ent : group)
    {
        auto renderSpec = group.get<RenderSpecComponent>(ent);
        auto transform = group.get<TransformComponent>(ent).transform();
        auto model = group.get<MeshComponent>(ent).model;
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
    lightsSSBO.unbind();
    hdrFBO.unbind();
    csmSSBO.unbind();

}

void RenderPipeline::bloomComputePass()
{
    // Bloom compute
    for(uint i=0; i<bloomSamples; i++)
    {
        //Downsample and blur
        downsampleAndBlur->bind();

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

        downsampleAndBlur->bindImage(bloomDownSampleTextures[i], 0, GL_WRITE_ONLY, false);
        downsampleAndBlur->dispatch((uint)ceil(size.x/16), (uint)ceil(size.y/16), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        downsampleAndBlur->unbind();
    }
    for(int i=bloomSamples-1; i>=0; i--)
    {
        // Upsample
        tentUpsampleAndAdd->bind();
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

        tentUpsampleAndAdd->bindImage(bloomUpSampleTextures[i], 0,  GL_WRITE_ONLY, false);
        tentUpsampleAndAdd->dispatch((uint)ceil(size.x/32), (uint)ceil(size.y/32), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        tentUpsampleAndAdd->unbind();
    }
}

void RenderPipeline::ssaoPass(std::shared_ptr<Scene> scene)
{
    auto sceneCamera = scene->activeCamera();
    ssaoFBO.bind();
    ssaoShader->bind();
    ssaoShader->setUniform("kernelSize", BufferElement::Int, config.ssaoKernelSize);
    ssaoShader->setUniform("radius", BufferElement::Float, config.ssaoRadius);
    ssaoShader->setUniform("bias", BufferElement::Float, config.ssaoBias);
    ssaoShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    hdrFBO.getDepthTex()->bind(0);
    ssaoNoiseTex->bind(1);
    ssaoKernelSSBO.bind();

    screenQuad.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();

    ssaoShader->unbind();
    ssaoKernelSSBO.unbind();
    ssaoFBO.unbind();
}

void RenderPipeline::ssaoBlur()
{
    simpleBlur->bind();
    blurFBO.bind();
    ssaoFBO.getTexture(0)->bind(0);
    simpleBlur->setUniform("kernelSize", BufferElement::Int, config.blurKernelSize);

    screenQuad.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();

    blurFBO.unbind();
    simpleBlur->unbind();
}

void RenderPipeline::compositePass()
{
    // Draw final ouput image
    outputFBO.bind();
    screenShader->bind();
    screenShader->setUniform("u_bloomIntensity", BufferElement::Float, config.bloomIntensity);
    hdrFBO.getTexture(0)->bind(0);
    blurFBO.getTexture(0)->bind(1);
    if(bloomUpSampleTextures.size()>0)
        bloomUpSampleTextures[0]->bind(2);

    screenQuad.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();

    screenShader->unbind();
    outputFBO.unbind();
}

void RenderPipeline::resizeOrClearResources()
{
    if(!config.enableBloom && bloomUpSampleTextures.size()>0)
        bloomUpSampleTextures[0]->clear({0,0,0});
    if(!config.enableSSAO)
        blurFBO.getTexture(0)->clear({1,1,1});
    // Resize FBOs and textures
    if(config.bloomRadius != oldBloomRadius)
    {
        resizeBloomBuffers();
        oldBloomRadius = config.bloomRadius;
    }
    if(winSize != oldWinSize)
    {
        hdrFBO.resize(winSize.x, winSize.y, 1);
        outputFBO.resize(winSize.x, winSize.y, 1);
        ssaoFBO.resize(winSize.x, winSize.y, 1);
        blurFBO.resize(winSize.x, winSize.y, 1);
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
        blurFBO.clear({1, 1, 1});

//        for(uint i=1; i<bloomSamples+1; i++)
//        {
//            bloomDownSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
//            bloomUpSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
//        }
    }
}

void RenderPipeline::drawSceneDebug(std::shared_ptr<Scene> scene)
{
    auto sceneCamera = scene->activeCamera();
    hdrFBO.bindColorAttachment(0);
    // Draw debug graphics
    BatchRenderer::begin(sceneCamera);
    scene->debugDraw();
    BatchRenderer::end();
    hdrFBO.unbind();
}

void RenderPipeline::drawScreenSpace(std::shared_ptr<Scene> scene)
{
    auto sceneCamera = scene->activeCamera();
    outputFBO.bind();
    BatchRenderer::begin(sceneCamera);
//    csmFBO.getDepthTex()->selectLayerForNextDraw(debugCascadeDisplayIndex);
//    auto size = (vec2)winSize/(float)bloomSamples;
//        for(uint i=0; i<bloomSamples; i++)
//        {
//            BatchRenderer::drawQuad({size.x*i,size.y}, size, bloomDownSampleTextures[i]);
//            BatchRenderer::drawQuad({size.x*i,size.y*2}, size, bloomUpSampleTextures[i]);
//        }
//    BatchRenderer::drawQuad({0,0}, winSize, bloomDownSampleTextures[0]);
    BatchRenderer::end();
    outputFBO.unbind();
}

void RenderPipeline::drawImgui()
{
    bool show = true;
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Render settings", &show))
    {
        // CSM
        TWEAK_BOOL("enableCSM", config.enableCSM);
        TWEAK_INT("shadowCascadeCount", config.shadowCascadeCount, 1, 0, 10);
        TWEAK_FLOAT("firstCascadeOffset", config.firstCascadeOffset, 0.01f);
        TWEAK_FLOAT("cascadeZextra", config.cascadeZextra, 0.01f);

        // BLOOM
        TWEAK_BOOL("enableBloom", config.enableBloom);
        TWEAK_FLOAT("bloomRadius", config.bloomRadius, 0.01f, 0, 8);
        TWEAK_FLOAT("bloomIntensity", config.bloomIntensity, 0.01f);
        TWEAK_FLOAT("bloomTreshold", config.bloomTreshold, 0.01f);
        TWEAK_FLOAT("exposure", config.exposure, 0.01f);

        // SSAO
        TWEAK_BOOL("enableSSAO", config.enableSSAO);
        TWEAK_INT("blurKernelSize", config.blurKernelSize, 2, 2, 20);
        TWEAK_INT("ssaoKernelSize", config.ssaoKernelSize, 2, 2, 256);
        TWEAK_FLOAT("ssaoRadius", config.ssaoRadius, 0.01f);
        TWEAK_FLOAT("ssaoBias", config.ssaoBias, 0.01f);


//        ImGui::ShowDemoWindow(&show);

        ImGui::End();
    }
}
