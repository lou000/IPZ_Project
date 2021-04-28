#include "meshrenderer.h"
#include "buffer.h"

void MeshRenderer::x_init()
{
    //Nothing for now...
}

void MeshRenderer::x_DrawMesh(const mat4 &model, const std::shared_ptr<MeshFile> &mesh, const vec4 &color)
{
    currentShader->bind();
    currentShader->setUniform("u_Model", Shader::Mat4, model);
    currentShader->setUniform("u_View", Shader::Mat4, currentCamera->getViewMatrix());
    currentShader->setUniform("u_Projection", Shader::Mat4, currentCamera->getProjMatrix());
    currentShader->setUniform("u_Color", Shader::Float4, color);
    currentShader->setUniform("u_CameraPosition", Shader::Float3, currentCamera->getPos());
    currentShader->setUniform("u_LightPosition", Shader::Float3, vec3{2.3, 3, 3});

    mesh->vertexArray()->bind();
    glDrawElements(GL_TRIANGLES, mesh->indexCount(), GL_UNSIGNED_SHORT, nullptr);
    mesh->vertexArray()->unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
    currentShader->unbind();
}

void MeshRenderer::x_DrawMesh(const vec3& pos, const vec3& size, const std::shared_ptr<MeshFile> &mesh, const vec4& color)
{
    auto model = translate(mat4(1.0f), pos) * scale(mat4(1.0f), size);
    x_DrawMesh(model, mesh, color);
}
