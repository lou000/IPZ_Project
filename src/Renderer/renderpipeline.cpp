#include "renderpipeline.h"
#include "../Core/application.h"


RenderPipeline::RenderPipeline()
{
    // Create main framebuffer
    FrameBufferAttachment colorAtt;
    colorAtt.type = GL_COLOR_ATTACHMENT0;
    colorAtt.format = GL_RGBA8;
    colorAtt.renderBuffer = true;

    FrameBufferAttachment depthAtt;
    depthAtt.type = GL_DEPTH_STENCIL_ATTACHMENT;
    depthAtt.format = GL_DEPTH24_STENCIL8;
    depthAtt.renderBuffer = true;
    auto winSize = App::getWindowSize();
    mainFBO = FrameBuffer(winSize.x, winSize.y, {colorAtt}, depthAtt, 16);


    // Create white texture for non-textured drawing
    whiteTexture = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTexture->setTextureData(&whiteData, sizeof(uint));

    // Create lights SSBO
    lightsSSBO = StorageBuffer(sizeof(uint) + MAX_LIGHTS*sizeof(PointLight), 2);
}


void RenderPipeline::drawScene(std::shared_ptr<Scene> scene)
{

    // DRAW pbr objects
    mainFBO.bind();
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
    glBindTexture(GL_TEXTURE_2D, 0);

    // Draw debug graphics
    BatchRenderer::begin(sceneCamera->getViewProjectionMatrix());
    scene->debugDraw();
    BatchRenderer::end();

    mainFBO.blitToFrontBuffer();


}
