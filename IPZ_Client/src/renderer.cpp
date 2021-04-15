#include "renderer.h"
#include "asset_manager.h"

extern "C" {    // this should help select dedicated gpu?
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void Renderer::x_init()
{
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    intermBuffer = (byte*)malloc(sizeof(byte)*MAX_VERTEX_BUFFER_SIZE);
}

void Renderer::x_begin(const std::string& renderable)
{
    glClear(GL_COLOR_BUFFER_BIT);
    currentRenderable = renderables[renderable];
    ASSERT(currentRenderable, "Renderer: Couldnt find renderable with name %s.", renderable.c_str());
    currentRenderable->onBegin();

    // it doesnt really belong here but we can always check by type later
    currentRenderable->shader()->setUniform("u_ViewProjection", Shader::Mat4, m_camera->getViewProjectionMatrix());
    startBatch();
}

void Renderer::x_end()
{
    flush();
}

void Renderer::startBatch()
{
    indexCount = 0;
    intermBufferPtr = intermBuffer;

}

void Renderer::nextBatch()
{
    flush();
    startBatch();
}

void Renderer::flush()
{
    if (indexCount == 0)
        return;

    uint size = (uint)((uint8*)intermBufferPtr - (uint8*)intermBuffer);
    currentRenderable->onFlush();
    currentRenderable->buffer()->setData(intermBuffer, size);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::x_setViewPort(uvec2 pos, uvec2 size)
{
    glViewport(pos.x, pos.y, size.x, size.y);
}

void Renderer::x_setClearColor(vec4 color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::x_addRenderable(std::shared_ptr<Renderable> renderable)
{
    renderables.insert({renderable->name(), renderable});
}

void Renderer::x_DrawQuad(const vec3& pos, const vec2& size, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, 0, size.y});
    x_DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer::x_DrawQuad(const vec3& pos, const vec2& size, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, 0, size.y});
    x_DrawQuad(transform, nullptr, 1, tintColor);
}

//should this be here or in TexturedQuad, idk either way feels wrong
void Renderer::x_DrawQuad(const mat4& transform, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{
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

    if (indexCount + 6 >= renderable->maxIndices())
        nextBatch();

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

    auto bPtr = (TexturedQuad::QuadVertex*) intermBufferPtr;
    for (size_t i = 0; i < 4; i++)
    {
        bPtr->position = transform * quadVertexPos[i];
        bPtr->color = tintColor;
        bPtr->texCoord = textureCoords[i];
        bPtr->texIndex = (float)textureIndex;
        bPtr->tilingFactor = tilingFactor;
        bPtr++;
    }
    intermBufferPtr = (byte*)bPtr;

    indexCount += 6;
}


void Renderer::x_DrawMesh(const vec3 &transform, const std::shared_ptr<MeshFile> &mesh, const vec4 &color)
{
    ASSERT(currentRenderable->type() == RenderableType::mesh,
           "Renderer: Renderables %s type is not a part of current rendering pass.",
           currentRenderable->name().c_str());
    UNUSED(transform);// just testing for now

    auto renderable = std::dynamic_pointer_cast<Mesh>(currentRenderable);
    if (indexCount + mesh->indexCount() >= renderable->maxIndices())
        nextBatch();

    memcpy(intermBufferPtr, mesh->vertices(), mesh->vertexCount()*sizeof (float));
    auto indexBuffer = std::make_shared<IndexBuffer>(mesh->indices(), mesh->indexCount());
    renderable->setIndexBuffer(indexBuffer);
    renderable->shader()->setUniform("u_Color", Shader::Float4, color);
}
