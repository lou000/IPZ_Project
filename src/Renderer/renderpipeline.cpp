#include "renderpipeline.h"
#include "../Core/application.h"


RenderPipeline::RenderPipeline()
{
    // Init shaders
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/final_composite.fs",
        "../assets/shaders/final_composite.vs"
    };
    screenShader = std::make_shared<Shader>("final_composite", shaderSrcs);
    AssetManager::addShader(screenShader);

    shaderSrcs = {
        "../assets/shaders/bloom_downsample.cmp"
    };
    downsampleAndBlur = std::make_shared<Shader>("bloom_downsample", shaderSrcs);
    AssetManager::addShader(downsampleAndBlur);

    shaderSrcs = {
        "../assets/shaders/bloom_upsample.cmp"
    };
    tentUpsampleAndAdd = std::make_shared<Shader>("bloom_upsample", shaderSrcs);
    AssetManager::addShader(tentUpsampleAndAdd);

    winSize = App::getWindowSize();

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

    hdrFBO = FrameBuffer(winSize.x, winSize.y, {colorAtt, bloomTresholdImage}, depthAtt, 0);

    // Create output framebuffer
    colorAtt.renderBuffer = true;
    outputFBO = FrameBuffer(winSize.x, winSize.y, {colorAtt}, {}, 0);

    // Create white texture for non-textured drawing
    whiteTexture = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTexture->setTextureData(&whiteData, sizeof(uint));

    // Create textures for bloom computation
//    bloomInputTex  = std::make_shared<Texture>(texSize.x, texSize.y, bloomDepth, GL_R11F_G11F_B10F, 1, true);
//    bloomOutputTex = std::make_shared<Texture>(texSize.x, texSize.y, bloomDepth, GL_R11F_G11F_B10F, 1, true);
    downSampleTextures.resize(BLOOM_SAMPLES);
    upSampleTextures.resize(BLOOM_SAMPLES);

    for(uint i=1; i<BLOOM_SAMPLES+1; i++)
    {
        uint width  = (float)winSize.x/(pow(2.f,i));
        uint height = (float)winSize.y/(pow(2.f,i));
        downSampleTextures[i-1] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, 1, true);
        LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
    }
    for(uint i=0; i<BLOOM_SAMPLES; i++)
    {
        uint width  = (float)winSize.x/(pow(2.f,i));
        uint height = (float)winSize.y/(pow(2.f,i));
        upSampleTextures[i] = std::make_shared<Texture>(width, height, 1, GL_R11F_G11F_B10F, 1, true);
        LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
    }

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

    // Create lights SSBO
    lightsSSBO = StorageBuffer(sizeof(uint) + MAX_LIGHTS*sizeof(PointLight), 2);
}


void RenderPipeline::drawScene(std::shared_ptr<Scene> scene)
{
    // Resize FBOs and textures
    if(winSize != App::getWindowSize())
    {
        winSize = App::getWindowSize();
        hdrFBO.resize(winSize.x, winSize.y);
        outputFBO.resize(winSize.x, winSize.y);

        for(uint i=1; i<BLOOM_SAMPLES+1; i++)
        {
            uint width  = (float)winSize.x/(pow(2.f,i));
            uint height = (float)winSize.y/(pow(2.f,i));
            downSampleTextures[i-1]->resize({width, height, 1});
            LOG("Size of bloom downsample %d: %d, %d\n", i, width, height);
        }
        for(uint i=0; i<BLOOM_SAMPLES; i++)
        {
            uint width  = (float)winSize.x/(pow(2.f,i));
            uint height = (float)winSize.y/(pow(2.f,i));
            upSampleTextures[i]->resize({width, height, 1});
            LOG("Size of bloom upsample %d: %d, %d\n", i, width, height);
        }
    }

    // Clear FBOs
    hdrFBO.clearColorAttachment(0, {0.302f, 0.345f, 0.388f});
    hdrFBO.clearColorAttachment(1, {0, 0, 0});
    hdrFBO.clearDepthAttachment({1, 1, 1});
    outputFBO.clear({1, 1, 1});
    for(uint i=1; i<BLOOM_SAMPLES+1; i++)
    {
        downSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
        upSampleTextures[i-1]->clear({0.969f, 0.353f, 0.580f});
    }

    // Clear textures, this can go away after debugging is done
//    bloomInputTex->clear({0.969f, 0.353f, 0.580f});
//    bloomOutputTex->clear({0.969f, 0.353f, 0.580f});

    // DRAW pbr objects
    hdrFBO.bind();
    auto sceneShader = scene->pbrShader;
    auto sceneCamera = scene->camera;
    sceneShader->bind();

    // Set camera data
    sceneShader->setUniform("u_View", BufferElement::Mat4, sceneCamera->getViewMatrix());
    sceneShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    sceneShader->setUniform("u_CameraPosition", BufferElement::Float3, sceneCamera->getPos());

    // Set directional light data
    sceneShader->setUniform("u_DirLightDirection", BufferElement::Float3, scene->skyLight.direction);
    sceneShader->setUniform("u_DirLightCol", BufferElement::Float3, scene->skyLight.color);
    sceneShader->setUniform("u_DirLightIntensity", BufferElement::Float, scene->skyLight.intensity);


    // Set point lights data
    std::vector<PointLight> enabledLights;
    for(auto light : scene->lights)
    {
        if(light.enabled)
            enabledLights.push_back(light);
    }
    uint lSize = (uint)enabledLights.size();
    sceneShader->setUniform("u_PointLightCount", BufferElement::Uint, lSize);
    lightsSSBO.bind();
    lightsSSBO.setData(enabledLights.data(), sizeof(PointLight)*lSize);


    for(auto ent : scene->entities)
    {
        // this is a lot of branching in a tight loop,
        // later we will store them separetly and go array after array
        if(ent.renderable && ent.enabled)
        {
            // draw here
            auto model = ent.getModelMatrix();
            sceneShader->setUniform("u_Model", BufferElement::Mat4, model);
            for(auto mesh : ent.model->meshes())
            {
                auto vao = mesh->vao();
                if(mesh->material.textured)
                {
                    //draw with textured_pbr
                    ASSERT_ERROR(0, "TODO: Textured pbr meshes my dude...");
                }
                else
                {
                    //draw with basic_pbr
                    sceneShader->setUniform("u_Metallic", BufferElement::Float, mesh->material.metallic);
                    sceneShader->setUniform("u_Roughness", BufferElement::Float, mesh->material.roughness);

                    //NOTE: We might add this as a boolean check but you shouldn't set override color to
                    //      invisible, instead you should set renderable to false
                    if(ent.overrideColor.a > 0)
                        sceneShader->setUniform("u_Color", BufferElement::Float4, ent.overrideColor);
                    else
                        sceneShader->setUniform("u_Color", BufferElement::Float4, mesh->material.color);
                    vao->bind();
                    glDrawElements(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, nullptr);
                    vao->unbind();
                }
            }
        }
    }
    lightsSSBO.unbind();
    hdrFBO.unbind();

    hdrFBO.bindColorAttachment(0);
    // Draw debug graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
//    scene->debugDraw();
    BatchRenderer::end();
    hdrFBO.unbind();

    // Bloom compute

    for(uint i=0; i<BLOOM_SAMPLES; i++)
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
            downSampleTextures[i-1]->bind(1);
            size = downSampleTextures[i-1]->getDimensions();
        }

        downsampleAndBlur->bindImage(downSampleTextures[i], 0, GL_WRITE_ONLY, false);
        downsampleAndBlur->dispatch((uint)ceil(size.x/16), (uint)ceil(size.y/16), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        downsampleAndBlur->unbind();
    }
    for(int i=BLOOM_SAMPLES-1; i>=0; i--)
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
            downSampleTextures[i-1]->bind(2);
            size = downSampleTextures[i-1]->getDimensions();
        }

        if(i==BLOOM_SAMPLES-1)
            downSampleTextures[i]->bind(1);
        else
            upSampleTextures[i+1]->bind(1);

        tentUpsampleAndAdd->bindImage(upSampleTextures[i], 0,  GL_WRITE_ONLY, false);
        tentUpsampleAndAdd->dispatch((uint)ceil(size.x/32), (uint)ceil(size.y/32), 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        tentUpsampleAndAdd->unbind();
    }


    // Draw final ouput image
    outputFBO.bind();
    screenShader->bind();
    hdrFBO.getTexture(0)->bind(0);
    upSampleTextures[0]->bind(1);

    screenQuad.bind();
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();
    screenShader->unbind();
    glEnable(GL_DEPTH_TEST);


    // Draw screenspace graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
    auto size = (vec2)winSize/(float)BLOOM_SAMPLES;

//    BatchRenderer::drawQuad({size.x,0}, size, downSampleTextures[5]);
//    BatchRenderer::drawQuad({0,0}, size, hdrFBO.getTexture(1));
//    for(uint i=0; i<BLOOM_SAMPLES; i++)
//    {
//        BatchRenderer::drawQuad({size.x*i,size.y}, size, downSampleTextures[i]);
//        BatchRenderer::drawQuad({size.x*i,size.y*2}, size, upSampleTextures[i]);
//    }
    BatchRenderer::end();

    outputFBO.blitToFrontBuffer();
    outputFBO.unbind();


}
