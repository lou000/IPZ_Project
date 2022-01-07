#include "renderpipeline.h"
#include "../Core/application.h"
#include "../Core/gui.h"

void RenderPipeline::initShaders()
{
    std::vector<ShaderFileDef> shaderSrcs = {
        {"../assets/shaders/basic_pbr.fs"},
        {"../assets/shaders/basic_pbr.vs"}
    };
    pbrShader = std::make_shared<Shader>("basic_pbr", shaderSrcs);
    AssetManager::addShader(pbrShader);

    shaderSrcs = {
        {"../assets/shaders/final_composite.fs"},
        {"../assets/shaders/final_composite.vs"}
    };
    screenShader = std::make_shared<Shader>("final_composite", shaderSrcs);
    AssetManager::addShader(screenShader);

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
        {"../assets/shaders/cascading_sm.gs", {"/*invocations*/"}, {", invocations = "+std::to_string(shadowCascadeCount+1)}},
        {"../assets/shaders/cascading_sm.vs"},
        {"../assets/shaders/passthrough.fs"}
    };
    csmShader = std::make_shared<Shader>("cascading_sm", shaderSrcs);
    AssetManager::addShader(csmShader);
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

    // Create output framebuffer
    colorAtt.renderBuffer = true;
    outputFBO = FrameBuffer(winSize.x, winSize.y, 1, {colorAtt}, {}, 0);

    FrameBufferAttachment smDepthAtt;
    smDepthAtt.type = GL_DEPTH_ATTACHMENT;
    smDepthAtt.format = GL_DEPTH_COMPONENT32F;
    smDepthAtt.renderBuffer = false;
    csmFBO = FrameBuffer(csmResolution, csmResolution, shadowCascadeCount+1, {}, smDepthAtt, 0);
}

void RenderPipeline::initSSBOs()
{
    // Create lights SSBO
    lightsSSBO = StorageBuffer(MAX_LIGHTS*sizeof(PointLight), 0);

    // Create light space matrix for DirLight
    csmSSBO = StorageBuffer(MAX_SHADOW_CASCADES*sizeof(mat4), 1);
}

void RenderPipeline::updateSSBOs(std::shared_ptr<Scene> scene)
{
    // Set point lights data
    std::vector<PointLight> enabledLights;
    for(auto light : scene->lights)
    {
        if(light.enabled)
            enabledLights.push_back(light);
    }

    enabledPointLightCount = (uint)enabledLights.size();
    lightsSSBO.setData(enabledLights.data(), sizeof(PointLight)*enabledPointLightCount);

    auto camera = scene->camera;

    std::vector<glm::mat4> cascades;

    cascades.resize(shadowCascadeCount+1);
    for (int i=0; i<shadowCascadeCount+1; ++i)
    {
        float cascade = 0;
        float prevCascade = 0;
        if(shadowCascadeCount == 1)
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
            else if (i<shadowCascadeCount)
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
        cascades[i] = calcDirLightViewProjMatrix(camera, scene->skyLight, prevCascade, cascade, debugCascadeZextra);
    }
    csmSSBO.setData(cascades.data(), sizeof(mat4)*cascades.size());
}

void RenderPipeline::updateCascadeRanges()
{
    cascadeRanges.resize(shadowCascadeCount);
    for (int i=0; i<shadowCascadeCount; ++i)
    {
        cascadeRanges[i] = (uint)glm::pow(3, i+1)+debugFirstCascadeOffset;
        LOG("Cascade %d: %f\n", i, cascadeRanges[i]);
    }
}

RenderPipeline::RenderPipeline()
{
    winSize = App::getWindowSize();

    initShaders();
    initFBOs();
    initSSBOs();
    updateCascadeRanges();

    // Create white texture for non-textured drawing
    whiteTexture = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTexture->setTextureData(&whiteData, sizeof(uint));

    // Initialize bloom buffers
    resizeBloomBuffers();

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


}


void RenderPipeline::drawScene(std::shared_ptr<Scene> scene)
{
    // Update runtime modified shaders
    if(shadowCascadeCount!=oldShadowCascadeCount)
    {
        ShaderFileDef def = {"../assets/shaders/cascading_sm.gs", {"/*invocations*/"}, {", invocations = "+std::to_string(shadowCascadeCount)}};
        csmShader->updateRuntimeModifiedStrings(def);
        oldShadowCascadeCount = shadowCascadeCount;

        LOG("Camera near:%f\n", scene->camera->getNearClip());
        updateCascadeRanges();
        LOG("Camera far:%f\n", scene->camera->getFarClip());
    }

    // Resize FBOs and textures
    if(bloomRadius != oldBloomRadius)
    {
        resizeBloomBuffers();
        oldBloomRadius = bloomRadius;
    }
    if(winSize != App::getWindowSize())
    {
        winSize = App::getWindowSize();
        hdrFBO.resize(winSize.x, winSize.y, 1);
        outputFBO.resize(winSize.x, winSize.y, 1);

        resizeBloomBuffers();
    }
    else
    {
        // Clear FBOs and textures if we didnt resize the window
        hdrFBO.clearColorAttachment(0, {0.302f, 0.345f, 0.388f});
        hdrFBO.clearColorAttachment(1, {0, 0, 0});
        hdrFBO.clearDepthAttachment();
        csmFBO.clearDepthAttachment();
        outputFBO.clear({1, 1, 1});

        for(uint i=1; i<bloomSamples+1; i++)
        {
            bloomDownSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
            bloomUpSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
        }
    }
    updateSSBOs(scene);

    //Sort entities
    std::vector<Entity> entitiesToDraw;
    for(auto& ent : scene->entities)
        if(ent.renderable && ent.enabled)
            entitiesToDraw.push_back(ent);

    // Draw cascading shadow maps to depth
    csmFBO.bind();
    csmSSBO.bind();
    csmShader->bind();
    glDisable(GL_BLEND);
    glCullFace(GL_FRONT);
    for(auto& ent : entitiesToDraw)
    {
        // draw here
        auto model = ent.getModelMatrix();
        csmShader->setUniform("u_Model", BufferElement::Mat4, model);
        for(auto mesh : ent.model->meshes())
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


    // DRAW pbr objects
    hdrFBO.bind();
    auto sceneCamera = scene->camera;
    pbrShader->bind();
    lightsSSBO.bind();

    // Set camera data
    pbrShader->setUniform("u_View", BufferElement::Mat4, sceneCamera->getViewMatrix());
    pbrShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    pbrShader->setUniform("u_CameraPosition", BufferElement::Float3, sceneCamera->getPos());

    // Set directional light data
    pbrShader->setUniform("u_DirLightDirection", BufferElement::Float3, scene->skyLight.direction);
    pbrShader->setUniform("u_DirLightCol", BufferElement::Float3, scene->skyLight.color);
    pbrShader->setUniform("u_DirLightIntensity", BufferElement::Float, scene->skyLight.intensity);
    pbrShader->setUniform("u_PointLightCount", BufferElement::Uint, enabledPointLightCount);

    // CSM data
    pbrShader->setUniform("u_farPlane", BufferElement::Float, scene->camera->getFarClip());
    pbrShader->setUniform("u_cascadeCount", BufferElement::Int, shadowCascadeCount+1);
    pbrShader->setUniformArray("u_cascadePlaneDistances", BufferElement::Float, cascadeRanges.data(), cascadeRanges.size());
    csmFBO.getDepthTex()->bind(3);



    for(auto& ent : entitiesToDraw)
    {
        // draw here
        auto model = ent.getModelMatrix();
        pbrShader->setUniform("u_Model", BufferElement::Mat4, model);
        for(auto mesh : ent.model->meshes())
        {
            auto vao = mesh->vao();
            // this below should probably go to separate draw call
            pbrShader->setUniform("u_bloomTreshold", BufferElement::Float, bloomTreshold);
            pbrShader->setUniform("u_exposure", BufferElement::Float, exposure);

            //draw with basic_pbr
            pbrShader->setUniform("u_Metallic", BufferElement::Float, mesh->material.metallic);
            pbrShader->setUniform("u_Roughness", BufferElement::Float, mesh->material.roughness);

            //NOTE: We might add this as a boolean check but you shouldn't set override color to
            //      invisible, instead you should set renderable to false
            if(ent.overrideColor.a > 0)
                pbrShader->setUniform("u_Color", BufferElement::Float4, ent.overrideColor);
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

    hdrFBO.bindColorAttachment(0);
    // Draw debug graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
//    scene->debugDraw();
    BatchRenderer::end();
    hdrFBO.unbind();

    // Bloom compute

    for(uint i=0; i<bloomSamples; i++)
    {
        //Downsample and blur
        //TODO: figure out how to calculate number of samples based on resolution
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

        if(i==bloomSamples-1)
            bloomDownSampleTextures[i]->bind(1);
        else
            bloomUpSampleTextures[i+1]->bind(1);

        tentUpsampleAndAdd->bindImage(bloomUpSampleTextures[i], 0,  GL_WRITE_ONLY, false);
        tentUpsampleAndAdd->dispatch((uint)ceil(size.x/32), (uint)ceil(size.y/32), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        tentUpsampleAndAdd->unbind();
    }


    // Draw final ouput image
    outputFBO.bind();
    screenShader->bind();
    screenShader->setUniform("u_bloomIntensity", BufferElement::Float, bloomIntensity);
    hdrFBO.getTexture(0)->bind(0);
    if(bloomUpSampleTextures.size()>0)
        bloomUpSampleTextures[0]->bind(1);

    screenQuad.bind();
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();
    screenShader->unbind();
    glEnable(GL_DEPTH_TEST);


    // Draw screenspace graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
    auto size = (vec2)winSize;

    csmFBO.getDepthTex()->selectLayerForNextDraw(debugCascadeDisplayIndex);
    BatchRenderer::drawQuad({0,0}, {500, 500}, csmFBO.getDepthTex());
    BatchRenderer::end();

    outputFBO.blitToFrontBuffer();
    outputFBO.unbind();

    bool show = true;
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Render settings", &show))
    {
        TWEAK_FLOAT(bloomRadius, 0.01f, 0, 8);
        TWEAK_FLOAT(bloomIntensity, 0.01f);
        TWEAK_FLOAT(bloomTreshold, 0.01f);
        TWEAK_FLOAT(exposure, 0.01f);
        TWEAK_INT(shadowCascadeCount, 1, 0, 10);
        TWEAK_INT(debugCascadeDisplayIndex, 1, 0, shadowCascadeCount+1);
        TWEAK_FLOAT(debugCascadeZextra, 0.01f);
        TWEAK_FLOAT(debugFirstCascadeOffset, 0.01f);

        ImGui::End();
    }

}

void RenderPipeline::resizeBloomBuffers()
{
    float minDim = min(winSize.x, winSize.y);
    float bSamples = (bloomRadius - 8.0f) + log(minDim) / log(2.f);
    bloomSamples = (int)bSamples < MAX_BLOOM_SAMPLES ? (int)bSamples : MAX_BLOOM_SAMPLES;

    uint prevSize = (uint)bloomDownSampleTextures.size();

    LOG("Resizing bloom buffers:\n    prevSize: %d, newSize: %d\n", prevSize, bloomSamples);

    bloomDownSampleTextures.resize(bloomSamples);
    bloomUpSampleTextures.resize(bloomSamples);

    // resize textures that are left after the resize
    uint size = prevSize>bloomSamples ? bloomSamples : prevSize;
    for(uint i=0; i<size; i++)
    {
        uint width  = (float)winSize.x/(pow(2.f,i+1));
        uint height = (float)winSize.y/(pow(2.f,i+1));
        bloomDownSampleTextures[i]->resize({width, height, 1});
        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=0; i<size; i++)
    {
        uint width  = (float)winSize.x/(pow(2.f,i));
        uint height = (float)winSize.y/(pow(2.f,i));
        bloomUpSampleTextures[i]->resize({width, height, 1});
        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }

    if(prevSize>=bloomSamples)
        return;

    // add new textures if the new size is bigger
    for(uint i=prevSize; i<bloomSamples; i++)
    {
        uint width  = (uint)((float)winSize.x/(float)(pow(2.f,i+1)));
        uint height = (uint)((float)winSize.y/(float)(pow(2.f,i+1)));
        bloomDownSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, 1, true);
        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=prevSize; i<bloomSamples; i++)
    {
        uint width  = (uint)((float)winSize.x/(float)(pow(2.f,i)));
        uint height = (uint)((float)winSize.y/(float)(pow(2.f,i)));
        bloomUpSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, 1, true);
        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }
}


