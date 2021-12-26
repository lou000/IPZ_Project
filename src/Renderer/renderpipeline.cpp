#include "renderpipeline.h"
#include "../Core/application.h"


RenderPipeline::RenderPipeline()
{
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
}

void RenderPipeline::drawScene(std::shared_ptr<Scene> scene)
{

    // DRAW pbr objects
    mainFBO.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    auto sceneShader = scene->pbrShader;
    auto sceneCamera = scene->camera;
    sceneShader->bind();
    sceneShader->setUniform("u_View", BufferElement::Mat4, sceneCamera->getViewMatrix());
    sceneShader->setUniform("u_Projection", BufferElement::Mat4, sceneCamera->getProjMatrix());
    sceneShader->setUniform("u_CameraPosition", BufferElement::Float3, sceneCamera->getPos());

    sceneShader->setUniform("u_DirLightPos", BufferElement::Float3, scene->directionalLight.pos);
    sceneShader->setUniform("u_DirLightCol", BufferElement::Float3, scene->directionalLight.color);


    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // TODO: remove test code
    for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    {
        glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime()) * 5.0, 0.0, 0.0);
//        newPos = lightPositions[i];
        auto str1 = "lightPositions[" + std::to_string(i) + "]";
        auto str2 = "lightColors[" + std::to_string(i) + "]";
        sceneShader->setUniform(str1.c_str(), BufferElement::Float3, newPos);
        sceneShader->setUniform(str2.c_str(), BufferElement::Float3, lightColors[i]);
    }

    // TODO: SSBO for pointlights
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
                    sceneShader->setUniform("u_Color", BufferElement::Float4, mesh->material.color);
                    vao->bind();
                    glDrawElements(GL_TRIANGLES, (GLsizei)vao->indexBuffer()->count(), GL_UNSIGNED_SHORT, nullptr);
                    vao->unbind();
                }
            }
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // TODO: DRAW debug graphics

    mainFBO.blitToFrontBuffer();


}
