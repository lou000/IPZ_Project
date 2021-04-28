#include "imrenderer.h"

extern "C" {    // this should help select dedicated gpu?
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void ImRender::x_init()
{
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);


    vertexBuffer = (byte*)malloc(MAX_VERTEX_BUFFER_SIZE);
    indexBuffer = (uint16*)malloc(MAX_INDEX_BUFFER_SIZE);
    vertexBufferEnd = vertexBuffer + MAX_VERTEX_BUFFER_SIZE/sizeof(byte);//redundant but clear
    indexBufferEnd  = indexBuffer + MAX_INDEX_BUFFER_SIZE/sizeof(uint16);
    vertexArray = std::make_shared<VertexArray>();
    vertexArray->bind();
    vertexArray->setIBuffer(std::make_shared<IndexBuffer>(MAX_INDEX_BUFFER_SIZE));
}

void ImRender::x_begin(const std::string& renderable)
{
    currentRenderable = renderables[renderable];
    ASSERT(currentRenderable, "Renderer: Couldnt find renderable with name %s.", renderable.c_str());
    currentRenderable->onBegin();
    vertexArray->addVBuffer(currentRenderable->buffer());
    // it doesnt really belong here but we can always check by type later
    currentRenderable->shader()->setUniform("u_ViewProjection", Shader::Mat4, m_camera->getViewProjectionMatrix());
    if(currentRenderable->type() == RenderSpecType::mesh)
    {
        currentRenderable->shader()->setUniform("u_CameraPosition", Shader::Float3, m_camera->getPos());
        currentRenderable->shader()->setUniform("u_LightPosition", Shader::Float3, vec3{2.3, 3, 3});
    }
    startBatch();
}

void ImRender::x_end()
{
    vertexArray->popVBuffer();
    flush();
}

void ImRender::startBatch()
{
    indexCount = 0;
    elementCount = 0;
    vertexBufferPtr = vertexBuffer;
    indexBufferPtr = indexBuffer;
}

void ImRender::nextBatch()
{
    flush();
    startBatch();
}

void ImRender::flush()
{
    if (indexCount == 0)
        return;

    currentRenderable->onFlush();
    uint sizeVB = (uint)((uint8*)vertexBufferPtr - (uint8*)vertexBuffer);
    uint sizeIB = (uint)((uint8*)indexBufferPtr - (uint8*)indexBuffer);
    currentRenderable->buffer()->setData(vertexBuffer, sizeVB);
    vertexArray->indexBuffer()->setData(indexBuffer, sizeIB);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ImRender::x_setViewPort(uvec2 pos, uvec2 size)
{
    glViewport(pos.x, pos.y, size.x, size.y);
}

void ImRender::x_setClearColor(vec4 color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void ImRender::x_addRenderable(std::shared_ptr<RenderSpec> renderable)
{
    renderables.insert({renderable->name(), renderable});
}

void ImRender::x_DrawQuad(const vec3& pos, const vec2& size, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, 0, size.y});
    x_DrawQuad(transform, texture, tilingFactor, tintColor);
}

void ImRender::x_DrawQuad(const vec3& pos, const vec2& size, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, 0, size.y});
    x_DrawQuad(transform, nullptr, 1, tintColor);
}

//should this be here or in TexturedQuad, idk either way feels wrong
void ImRender::x_DrawQuad(const mat4& transform, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{
    if (indexBufferPtr+6 >= indexBufferEnd || vertexBufferPtr + 64 >= vertexBufferEnd)
        nextBatch();

    ASSERT(currentRenderable->type() == texturedQuad,
           "Renderer: Renderables %s type is not a part of current rendering pass.",
           currentRenderable->name().c_str());

    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    auto renderable = std::dynamic_pointer_cast<TexturedQuad>(currentRenderable);


    int textureIndex = 0;
    if(texture == nullptr)
        textureIndex = 0;
    else
    {
        textureIndex = renderable->addTexture(texture);
        if(textureIndex == 0)
        {
            nextBatch();
            textureIndex = renderable->addTexture(texture);
        }
    }

    //VERTICES
    const vec4 quadVertexPos[4] =
    {
        {-0.5f,  0.0f,  0.5f, 1.0f},
        { 0.5f,  0.0f,  0.5f, 1.0f},
        { 0.5f,  0.0f, -0.5f, 1.0f},
        {-0.5f,  0.0f, -0.5f, 1.0f}
    };
    auto bPtr = (TexturedQuad::QuadVertex*) vertexBufferPtr;
    for (size_t i = 0; i < 4; i++)
    {
        bPtr->position = transform * quadVertexPos[i];
        bPtr->color = tintColor;
        bPtr->texCoord = textureCoords[i];
        bPtr->texIndex = (float)textureIndex;
        bPtr->tilingFactor = tilingFactor;
        bPtr++;
    }
    vertexBufferPtr = (byte*)bPtr;

    //INDICES
    indexBufferPtr[0] = elementCount + 0;
    indexBufferPtr[1] = elementCount + 1;
    indexBufferPtr[2] = elementCount + 2;

    indexBufferPtr[3] = elementCount + 2;
    indexBufferPtr[4] = elementCount + 3;
    indexBufferPtr[5] = elementCount + 0;

    indexBufferPtr+=6;

    indexCount += 6;
    elementCount+=4;
}

void ImRender::x_DrawMesh(const vec3& pos, const vec3& size, const std::shared_ptr<MeshFile> &mesh, const vec4& color)
{
    x_DrawMesh(translate(mat4(1.0f), pos), mat4(1), scale(mat4(1.0f), size), mesh, color);
}

void ImRender::x_DrawMesh(const mat4& translation, const mat4& rotation, const mat4& scale, const std::shared_ptr<MeshFile> &mesh, const vec4 &color)
{
    ASSERT(currentRenderable->type() == RenderSpecType::mesh,
           "Renderer: Renderables %s type is not a part of current rendering pass.",
           currentRenderable->name().c_str());

    if (indexBufferPtr+mesh->indexCount() >= indexBufferEnd ||
        vertexBufferPtr + (mesh->vertexCount()*mesh->stride()) >= vertexBufferEnd)
        nextBatch();

    auto renderable = std::dynamic_pointer_cast<ColoredMesh>(currentRenderable);

    if(renderable->color()!=color && renderable->color() != vec4(0,0,0,0))
        nextBatch();
    renderable->setColor(color);

    auto vertices = (ColoredMesh::MeshVertex*) mesh->vertices();
    auto bPtr = (ColoredMesh::MeshVertex*) vertexBufferPtr;

    auto transform = translation * rotation * scale;
    for(uint i=0; i<mesh->vertexCount(); i+=1)
    {
        bPtr->position = transform * vec4(vertices->position, 1);
        bPtr->normals = rotation * vec4(vertices->normals, 1);
        bPtr++;
        vertices++;
    }
    vertexBufferPtr = (byte*)bPtr;

    //For now we increment indices, we'll see what to do later
    uint16* indices = mesh->indices();
    for(uint i=0; i<mesh->indexCount(); i++)
    {
        *indexBufferPtr = indices[i]+elementCount;
        indexBufferPtr++;
    }

    //bug right here
    elementCount+=mesh->vertexCount();
    indexCount+=mesh->indexCount();
}
