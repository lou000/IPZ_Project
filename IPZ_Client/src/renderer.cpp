#include "renderer.h"
#include "asset_manager.h"

extern "C" {    // this should help select dedicated gpu?
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void Renderer::x_init()
{
    // For now this class is specific to rendering quads with one shader,
    // if we require multiple shaders, or multiple batches for geometry
    // we should use framebuffers


    //TODO: Rewrite this so it doesnt use QuadVertex, just creates void* memory block
    //      We add everything to buffer according to layout.
    //      Make renderer take buffer when starting batch and add shader as buffer member.
    //      Maybe make vertex buffer identifyable by shader.
    //      All this hopefully enables us to render using diffirent shaders, and finally use depth buffer
    //
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

    if (indexCount + 6 >= renderable->maxIndices)
        nextBatch();

    int textureIndex = 0;
    if(texture == nullptr)
    {
        textureIndex = 0;
    }
    else
    {
        //dude make something else for this
        for (uint32_t i = 1; i < renderable->textureCount; i++)
        {
            if (renderable->textureSlots[i]->id() == texture->id())
            {
                textureIndex = i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (renderable->textureCount >= renderable->textureSlots.size())
                nextBatch();

            textureIndex = renderable->textureCount;
            renderable->textureSlots[renderable->textureCount] = texture;
            renderable->textureCount++;
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

