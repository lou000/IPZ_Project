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

    auto winSize = App::getWindowSize();
    hdrFBO = FrameBuffer(winSize.x, winSize.y, {colorAtt, bloomTresholdImage}, depthAtt, 0);

    // Create output framebuffer
    colorAtt.renderBuffer = true;
    outputFBO = FrameBuffer(winSize.x, winSize.y, {colorAtt}, {}, 16);

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

    // Create lights SSBO
    lightsSSBO = StorageBuffer(sizeof(uint) + MAX_LIGHTS*sizeof(PointLight), 2);
}


void RenderPipeline::drawScene(std::shared_ptr<Scene> scene)
{

    // DRAW pbr objects
    hdrFBO.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

    // Draw debug graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
    scene->debugDraw();
    BatchRenderer::end();
    hdrFBO.unbind();


    // Draw final ouput image
    outputFBO.bind();
    screenShader->bind();
    hdrFBO.getTexture(0)->bind(0);

    screenQuad.bind();
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    screenQuad.unbind();
    screenShader->unbind();
    glEnable(GL_DEPTH_TEST);

    // Draw screenspace graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
    auto size = (vec2)App::getWindowSize()/5.f;
    BatchRenderer::drawQuad({0,0}, size, hdrFBO.getTexture(1));
    BatchRenderer::drawQuad({size.x,0}, size, hdrFBO.getTexture(0));
    BatchRenderer::end();

    outputFBO.blitToFrontBuffer();
    outputFBO.unbind();


}
