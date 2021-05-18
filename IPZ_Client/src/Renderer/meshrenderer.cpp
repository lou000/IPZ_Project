#include "meshrenderer.h"

void MeshRenderer::x_init()
{
    //Nothing for now...
}

void MeshRenderer::x_begin()
{
    auto currentCamera = GraphicsContext::getCamera();
    currentShader->bind();
    currentShader->setUniform("u_View", Shader::Mat4, currentCamera->getViewMatrix());
    currentShader->setUniform("u_Projection", Shader::Mat4, currentCamera->getProjMatrix());
    currentShader->setUniform("u_CameraPosition", Shader::Float3, currentCamera->getPos());
    currentShader->setUniform("u_LightPosition", Shader::Float3, vec3{2.3, 3, 3});
}

void MeshRenderer::x_end()
{
    currentShader->unbind();
}

void MeshRenderer::x_drawMesh(const mat4 &model, const std::shared_ptr<MeshFile> &mesh, const vec4 &color)
{
    currentShader->setUniform("u_Model", Shader::Mat4, model);
    currentShader->setUniform("u_Color", Shader::Float4, color);

    mesh->vertexArray()->bind();
    glDrawElements(GL_TRIANGLES, mesh->indexCount(), GL_UNSIGNED_SHORT, nullptr);
    mesh->vertexArray()->unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MeshRenderer::x_drawMesh(const vec3& pos, const vec3& size, const std::shared_ptr<MeshFile> &mesh, const vec4& color)
{
    auto model = translate(mat4(1.0f), pos) * scale(mat4(1.0f), size);
    x_drawMesh(model, mesh, color);
}

void MeshRenderer::x_generateCubeSphere(const vec3 &pos, int vPerEdge, float radius, const vec4 &color)
{
    ASSERT(vPerEdge>=2);

    // Vertices
    float offset = (float)1/(vPerEdge-1);
    int cornerVertices = 8;
    int edgeVertices = (vPerEdge-2) * 12;
    int faceVertices = (vPerEdge-2)*(vPerEdge-2)*6;
    size_t vCount = cornerVertices + edgeVertices + faceVertices;
    vec3* posVerts = (vec3*)alloca(vCount*sizeof(vec3));

    size_t v = 0;
    for(int y = 0; y<vPerEdge; y++)
    {
        // front row
        for (int i = 0; i < vPerEdge; i++)
            posVerts[v++] = vec3(-0.5+i*offset, -0.5+y*offset, 0.5);

        // right row
        for (int i = 1; i < vPerEdge; i++)
            posVerts[v++] = vec3(0.5, -0.5+y*offset, 0.5-i*offset);

        // back row
        for (int i = 1; i < vPerEdge; i++)
            posVerts[v++] = vec3(0.5-i*offset, -0.5+y*offset, -0.5);

        // left row
        for (int i = 1; i < vPerEdge-1; i++)
            posVerts[v++] = vec3(-0.5, -0.5+y*offset, -0.5+i*offset);
    }
    // top face
    for (int z = 1; z < vPerEdge-1; z++)
        for (int x = 1; x < vPerEdge-1; x++)
            posVerts[v++] = vec3(-0.5+x*offset,  0.5, 0.5-z*offset);

    // bottom face
    for (int z = 1; z < vPerEdge-1; z++)
        for (int x = 1; x < vPerEdge-1; x++)
            posVerts[v++] = vec3(-0.5+x*offset, -0.5, 0.5-z*offset);




}
