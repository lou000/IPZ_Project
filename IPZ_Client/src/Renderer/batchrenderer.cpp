﻿#include "batchrenderer.h"
#include "gtx/vector_angle.hpp"

extern "C" {    // this should help select dedicated gpu?
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void BatchRenderer::x_init()
{
    // get max texture count and setup storage
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);
    ASSERT(maxTextureSlots>0);
    texSamplers = (int*)malloc(maxTextureSlots * sizeof(int));
    for(int i=0; i<maxTextureSlots; i++)
        texSamplers[i] = i;
    textureSlots.resize(maxTextureSlots, nullptr);

    // setup white texture used for colored, untextured drawing
    whiteTex = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTex->setTextureData(&whiteData, sizeof(uint));
    textureSlots[0] = whiteTex;

    // setup buffers
    vertexBuffer = (byte*)malloc(MAX_VERTEX_BUFFER_SIZE);
    indexBuffer = (uint16*)malloc(MAX_INDEX_BUFFER_SIZE);
    vertexBufferEnd = vertexBuffer + MAX_VERTEX_BUFFER_SIZE/sizeof(byte);//redundant but clear
    indexBufferEnd  = indexBuffer + MAX_INDEX_BUFFER_SIZE/sizeof(uint16);

    BufferLayout layout = {
        {Shader::Float4, "a_Position"    },
        {Shader::Float4, "a_Color"       },
        {Shader::Float2, "a_TexCoord"    },
        {Shader::Float , "a_TexIndex"    },
        {Shader::Float , "a_TilingFactor"}
    };
    auto iBuffer = std::make_shared<IndexBuffer>(MAX_INDEX_BUFFER_SIZE);
    auto vBuffer = {std::make_shared<VertexBuffer>(layout, MAX_VERTEX_BUFFER_SIZE)};
    vertexArray = std::make_shared<VertexArray>(vBuffer, iBuffer);
}

void BatchRenderer::x_begin()
{
    // bind all uniforms
//    glDisable(GL_CULL_FACE);
    m_currentShader->bind();
    m_currentShader->setUniformArray("u_Textures", Shader::Int, texSamplers, maxTextureSlots);

    // setup projections
    viewProj3d = GraphicsContext::getCamera()->getViewProjectionMatrix();
    m_currentShader->setUniform("u_ViewProjection", Shader::Mat4, viewProj3d);
    auto viewSize = GraphicsContext::getViewPortSize();
    viewProjOrtho = glm::ortho(0.f, (float)viewSize.x, (float)viewSize.y, 0.f, -1.f, 1.f)*glm::lookAt(vec3(0,0,1),vec3(0,0,0),vec3(0,1,0));

    vertexArray->bind();
    startBatch();
}

void BatchRenderer::x_end()
{
    flush();
    m_currentShader->unbind();
    vertexArray->unbind();
}

int BatchRenderer::addTexture(const std::shared_ptr<Texture>& texture)
{
    int textureIndex = 0;
    for (int i = 1; i < textureCount; i++)
    {
        if (textureSlots[i]->id() == texture->id()) // texture is in use
        {
            textureIndex = i;
            break;
        }
    }
    if(textureIndex == 0) // texture was not already in use
    {
        if (textureCount >= maxTextureSlots) // if we exceed maxTextureSlots
            nextBatch();
        textureIndex = textureCount;
        textureSlots[textureCount] = texture;
        textureCount++;
    }
    return textureIndex;
}

void BatchRenderer::startBatch()
{
    // reset everything to starting values
    indexCount      = 0;
    elementCount    = 0;
    textureCount    = 1;
    vertexBufferPtr = vertexBuffer;
    indexBufferPtr  = indexBuffer;
}

void BatchRenderer::nextBatch()
{
    flush();
    startBatch();
}

void BatchRenderer::flush()
{
    if (indexCount == 0) // if we didnt submit anything
        return;

    // bind all the textures
    for (uint32_t i = 0; i < textureSlots.size(); i++)
        if(textureSlots[i])
            textureSlots[i]->bind(i);

    // set buffers data
    uint sizeVB = (uint)((uint8*)vertexBufferPtr - (uint8*)vertexBuffer);
    uint sizeIB = (uint)((uint8*)indexBufferPtr - (uint8*)indexBuffer);
    vertexArray->vertexBuffers().front()->setData(vertexBuffer, sizeVB);
    vertexArray->indexBuffer()->setData(indexBuffer, sizeIB);

    // draw
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BatchRenderer::x_drawQuad(const vec3& pos, const vec2& size, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, 0, size.y});
    x_drawQuad(transform, texture, tilingFactor, tintColor);
}

void BatchRenderer::x_drawQuad(const vec3& pos, const vec2& size, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, 0, size.y});
    x_drawQuad(transform, nullptr, 1, tintColor);
}

void BatchRenderer::x_drawQuad(const mat4& transform, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{
    // if we would go outside the bounds of the buffers do the next batch
    if (indexBufferPtr+6 >= indexBufferEnd || vertexBufferPtr + 64 >= vertexBufferEnd)
        nextBatch();

    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    int textureIndex = 0;
    if(texture == nullptr)
        textureIndex = 0;                   // use white texture
    else
        textureIndex = addTexture(texture); // add new texture

    //VERTICES
    const vec4 quadVertexPos[4] =
    {
        {-0.5f,  0.0f,  0.5f, 1.0f},
        { 0.5f,  0.0f,  0.5f, 1.0f},
        { 0.5f,  0.0f, -0.5f, 1.0f},
        {-0.5f,  0.0f, -0.5f, 1.0f}
    };
    auto bPtr = (QuadVertex*) vertexBufferPtr;

    auto mvp = viewProj3d * transform;

    for (size_t i = 0; i < 4; i++)
    {
        bPtr->position = mvp * quadVertexPos[i];
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

void BatchRenderer::x_drawLine(const vec2 &posStart, const vec2 &posEnd, float width, const vec4& color)
{
    //Line always faces camera
    auto lineVec = normalize(posStart-posEnd);
    auto perpVec = normalize(vec2(lineVec.y, -lineVec.x));
    auto offset = perpVec*(width/2);

        const vec4 lineVertexPos[4] =
            {
                vec4(posStart - offset, 0, 1),
                vec4(posStart + offset, 0, 1),
                vec4(posEnd   + offset, 0, 1),
                vec4(posEnd   - offset, 0, 1),
            };

    //The rest of it is the quad
    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    auto bPtr = (QuadVertex*) vertexBufferPtr;
    for (size_t i = 0; i < 4; i++)
    {
        bPtr->position = viewProjOrtho * lineVertexPos[i];
        bPtr->color = color;
        bPtr->texCoord = textureCoords[i];
        bPtr->texIndex = 0;
        bPtr->tilingFactor = 1;
        bPtr++;
    }
    vertexBufferPtr = (byte*)bPtr;

    indexBufferPtr[0] = elementCount + 0;
    indexBufferPtr[1] = elementCount + 1;
    indexBufferPtr[2] = elementCount + 3;

    indexBufferPtr[3] = elementCount + 3;
    indexBufferPtr[4] = elementCount + 1;
    indexBufferPtr[5] = elementCount + 2;

    indexBufferPtr+=6;

    indexCount += 6;
    elementCount+=4;
}

void BatchRenderer::x_drawLine(const vec3 &posStart, const vec3 &posEnd, float width, const vec4& color)
{
    x_drawLine_internal(viewProj3d, posStart, posEnd, width, color);
}

void BatchRenderer::x_drawLine_internal(const mat4 proj, const vec3 &posStart, const vec3 &posEnd, float width, const vec4& color)
{
    //Line always faces camera
    auto lineVec = normalize(posStart-posEnd);
    auto cameraForward = GraphicsContext::getCamera()->forward();
    auto crossP = cross(cameraForward, lineVec);
    auto offset = crossP*(width/2);

    const vec4 lineVertexPos[4] =
    {
        vec4(posStart + offset, 1),
        vec4(posStart - offset, 1),
        vec4(posEnd   - offset, 1),
        vec4(posEnd   + offset, 1),
    };

    //The rest of it is the quad
    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    auto bPtr = (QuadVertex*) vertexBufferPtr;
    for (size_t i = 0; i < 4; i++)
    {
        bPtr->position = proj * lineVertexPos[i];
        bPtr->color = color;
        bPtr->texCoord = textureCoords[i];
        bPtr->texIndex = 0;
        bPtr->tilingFactor = 1;
        bPtr++;
    }
    vertexBufferPtr = (byte*)bPtr;

    indexBufferPtr[0] = elementCount + 0;
    indexBufferPtr[1] = elementCount + 1;
    indexBufferPtr[2] = elementCount + 3;

    indexBufferPtr[3] = elementCount + 3;
    indexBufferPtr[4] = elementCount + 1;
    indexBufferPtr[5] = elementCount + 2;

    indexBufferPtr+=6;

    indexCount += 6;
    elementCount+=4;
}
