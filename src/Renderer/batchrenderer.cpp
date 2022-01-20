#include "batchrenderer.h"
#include "gtx/vector_angle.hpp"
#include "../AssetManagement/asset_manager.h"

extern "C" {    // this should help select dedicated gpu?
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void BatchRenderer::x_init()
{
    // get max texture count and setup storage
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);
    ASSERT(maxTextureSlots>0);
    maxTextureSlots = maxTextureSlots-5;
    maxTextureSlotsArray = 5;
    texSamplers = (int*)malloc(maxTextureSlots * sizeof(int));
    for(int i=0; i<maxTextureSlots; i++)
        texSamplers[i] = i;

    texSamplersArray = (int*)malloc(maxTextureSlotsArray * sizeof(int));
    for(int i=0; i<maxTextureSlotsArray; i++)
        texSamplersArray[i] = i+27;

    textureSlots.resize(maxTextureSlots+maxTextureSlotsArray);

    // setup white texture used for colored, untextured drawing
    whiteTex = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTex->setData(&whiteData, sizeof(uint));
    textureSlots[0] = {whiteTex, 0};

    // setup buffers
    vertexBuffer = (byte*)malloc(MAX_VERTEX_BUFFER_SIZE);
    indexBuffer = (uint16*)malloc(MAX_INDEX_BUFFER_SIZE);
    vertexBufferEnd = vertexBuffer + MAX_VERTEX_BUFFER_SIZE/sizeof(byte);
    indexBufferEnd  = indexBuffer  + MAX_INDEX_BUFFER_SIZE /sizeof(uint16);

    BufferLayout layout = {
        {BufferElement::Float4, "a_Position"    },
        {BufferElement::Float4, "a_Color"       },
        {BufferElement::Float2, "a_TexCoord"    },
        {BufferElement::Float , "a_TexLayer"    },
        {BufferElement::Float , "a_TexIndex"    },
        {BufferElement::Float , "a_TilingFactor"}
    };
    auto iBuffer = std::make_shared<IndexBuffer>(MAX_INDEX_BUFFER_SIZE);
    auto vBuffer = {std::make_shared<VertexBuffer>(layout, MAX_VERTEX_BUFFER_SIZE)};
    vertexArray = std::make_shared<VertexArray>(vBuffer, iBuffer);

    std::vector<ShaderFileDef> shaderSrcs2 = {
        {"../assets/shaders/default_batch.fs"},
        {"../assets/shaders/default_batch.vs"}
    };
    m_debugShader = std::make_shared<Shader>("batch", shaderSrcs2);
    AssetManager::addShader(m_debugShader);
}

void BatchRenderer::x_begin(std::shared_ptr<Camera> camera)
{
    // bind all uniforms
//    glDisable(GL_CULL_FACE);
    if(camera)
        viewProj3d = camera->getViewProjectionMatrix();
    currentCamera = camera;
    m_debugShader->bind();
    m_debugShader->setUniformArray("u_Textures", BufferElement::Int, texSamplers, maxTextureSlots);
    m_debugShader->setUniformArray("u_TextureArrays", BufferElement::Int, texSamplersArray, maxTextureSlotsArray);

    // setup projections
    m_debugShader->setUniform("u_ViewProjection", BufferElement::Mat4, viewProj3d);
    auto viewSize = GraphicsContext::getViewPortSize();
    viewProjOrtho = glm::ortho(0.f, (float)viewSize.x, (float)viewSize.y, 0.f, -1.f, 1.f)*glm::lookAt(vec3(0,0,1),vec3(0,0,0),vec3(0,1,0));

    vertexArray->bind();
    startBatch();
}

void BatchRenderer::x_end()
{
    flush();
    m_debugShader->unbind();
    vertexArray->unbind();
    currentCamera = nullptr;
}

int BatchRenderer::addTexture(const std::shared_ptr<Texture>& texture)
{
    int textureIndex = 0;
    for (int i = 1; i < maxTextureSlotsArray + maxTextureSlots; i++)
    {
        auto tex = textureSlots[i].texture;
        if (tex && tex->id() == texture->id()) // texture is in use
        {
            textureIndex = i;
            break;
        }
    }
    if(textureIndex == 0) // texture was not already in use
    {
        if (textureCount >= maxTextureSlots || textureCountArray >= maxTextureSlotsArray) // if we exceed maxTextureSlots
            nextBatch();
        if(texture->getDimensions().z>1)
        {
            textureIndex = maxTextureSlots+textureCountArray;
            textureCountArray++;
        }
        else
        {
            textureIndex = textureCount;
            textureCount++;
        }
        textureSlots[textureIndex] = {texture, texture->selectedLayer()};
    }
    return textureIndex;
}

void BatchRenderer::startBatch()
{
    // reset everything to starting values
    indexCount      = 0;
    elementCount    = 0;
    textureCount    = 1;
    textureCountArray = 0;
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

    //bind all the textures
    for (uint32_t i = 0; i < textureSlots.size(); i++)
    {
        auto textureSlot = textureSlots[i];
        if(textureSlot.texture)
        {
            textureSlot.texture->bind(i);
        }
    }

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



void BatchRenderer::x_drawQuad(const vec2& pos, const vec2& size, const vec4& tintColor)
{
    const vec4 quadVertexPos[4] =
    {
        viewProjOrtho * vec4(pos.x,        pos.y+size.y, 1, 1),
        viewProjOrtho * vec4(pos.x+size.x, pos.y+size.y, 1, 1),
        viewProjOrtho * vec4(pos.x+size.x, pos.y,        1, 1),
        viewProjOrtho * vec4(pos.x,        pos.y,        1, 1),
    };
    x_drawQuad_internal(quadVertexPos, nullptr, 1, tintColor);
}

void BatchRenderer::x_drawQuad(const vec2& pos, const vec2& size, const std::shared_ptr<Texture>& texture,
                               float tilingFactor, const vec4& tintColor)
{
    const vec4 quadVertexPos[4] =
    {
        viewProjOrtho * vec4(pos.x,        pos.y+size.y, 1, 1),
        viewProjOrtho * vec4(pos.x+size.x, pos.y+size.y, 1, 1),
        viewProjOrtho * vec4(pos.x+size.x, pos.y,        1, 1),
        viewProjOrtho * vec4(pos.x,        pos.y,        1, 1),
    };
    x_drawQuad_internal(quadVertexPos, texture, tilingFactor, tintColor);
}

void BatchRenderer::x_drawQuad(const mat4& transform, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{

    auto mvp = viewProj3d * transform;
    const vec4 quadVertexPos[4] =
    {
        mvp * vec4(-0.5f,  0.0f,  0.5f, 1.0f),
        mvp * vec4( 0.5f,  0.0f,  0.5f, 1.0f),
        mvp * vec4( 0.5f,  0.0f, -0.5f, 1.0f),
        mvp * vec4(-0.5f,  0.0f, -0.5f, 1.0f),
    };

    x_drawQuad_internal(quadVertexPos, texture, tilingFactor, tintColor);
}

void BatchRenderer::x_drawLine(const vec2 &posStart, const vec2 &posEnd, float width, const vec4& color)
{
    auto lineVec = normalize(posStart-posEnd);
    auto perpVec = normalize(vec2(lineVec.y, -lineVec.x));
    auto offset = perpVec*(width/2);

    const vec4 lineVertexPos[4] =
    {
        viewProjOrtho * vec4(posStart - offset, 1, 1),
        viewProjOrtho * vec4(posStart + offset, 1, 1),
        viewProjOrtho * vec4(posEnd   + offset, 1, 1),
        viewProjOrtho * vec4(posEnd   - offset, 1, 1),
    };

    x_drawQuad_internal(lineVertexPos, nullptr, 1, color);
}

void BatchRenderer::x_drawLine(const vec3 &posStart, const vec3 &posEnd, float width, const vec4& color)
{
    //Line always faces camera
    auto lineVec = normalize(posStart-posEnd);
    auto cameraForward = currentCamera->forward();
    auto crossP = normalize(cross(cameraForward, lineVec));
    auto offset = crossP*(width/2);

    const vec4 lineVertexPos[4] =
    {
        viewProj3d * vec4(posStart + offset, 1),
        viewProj3d * vec4(posStart - offset, 1),
        viewProj3d * vec4(posEnd   - offset, 1),
        viewProj3d * vec4(posEnd   + offset, 1),
    };

    x_drawQuad_internal(lineVertexPos, nullptr, 1, color);

}

void BatchRenderer::x_drawPoint(const vec3 &pos, float lWidth, float lLen, const vec4 &color)
{
    BatchRenderer::drawLine(pos-vec3(lLen/2, 0, 0), pos+vec3(lLen/2, 0, 0), lWidth, color);
    BatchRenderer::drawLine(pos-vec3(0, lLen/2, 0), pos+vec3(0, lLen/2, 0), lWidth, color);
    BatchRenderer::drawLine(pos-vec3(0, 0, lLen/2), pos+vec3(0, 0, lLen/2), lWidth, color);
}

void BatchRenderer::x_drawTris(vec3 *verts, uint16 *indices, uint iCount, float lWidth, const vec4 &color)
{
    for(size_t i=0; i<iCount; i+=6)
    {
        BatchRenderer::drawLine(verts[indices[i]],   verts[indices[i+1]], lWidth, color);
        BatchRenderer::drawLine(verts[indices[i+1]], verts[indices[i+2]], lWidth, color);
        BatchRenderer::drawLine(verts[indices[i+2]], verts[indices[i+0]], lWidth, color);

        BatchRenderer::drawLine(verts[indices[i+3]], verts[indices[i+4]], lWidth, color);
        BatchRenderer::drawLine(verts[indices[i+4]], verts[indices[i+5]], lWidth, color);
        BatchRenderer::drawLine(verts[indices[i+5]], verts[indices[i+3]], lWidth, color);
    }
}

void BatchRenderer::x_drawCircle(const vec2 &pos, float radius, int triangles, const vec4 &color)
{
    // if we would go outside the bounds of the buffers do the next batch
    if (indexBufferPtr+(triangles+1)*3 >= indexBufferEnd || vertexBufferPtr + (triangles+1)*sizeof(QuadVertex) >= vertexBufferEnd)
        nextBatch();

    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };


    auto bPtr = (QuadVertex*) vertexBufferPtr;
    float angle = 0.f;
    float increment = 2.0f * glm::pi<float>() / triangles;

    for(size_t i=0; i<(size_t)triangles+1; i++)
    {
        vec4 vPos;
        if(i==0)
        {
            vPos = viewProjOrtho*vec4{pos.x, pos.y, 1, 1};
        }
        else
        {
            vPos = viewProjOrtho*vec4(radius*cos(angle)+pos.x, radius*sin(angle)+ pos.y, 1, 1);
            angle += increment;
        }
        bPtr->position = vPos;
        bPtr->color = color;
        bPtr->texCoord = textureCoords[i];
        bPtr->texIndex = 0;
        bPtr->tilingFactor = 1;
        bPtr++;
    }
    vertexBufferPtr = (byte*)bPtr;

    for(int i=0; i<triangles; i++)
    {
        indexBufferPtr[0] = elementCount + 0;
        indexBufferPtr[1] = elementCount + i+1;
        indexBufferPtr[2] = elementCount + i;
        indexBufferPtr += 3;
    }
    indexBufferPtr[0] = elementCount + 0;
    indexBufferPtr[1] = elementCount + 1;
    indexBufferPtr[2] = elementCount + triangles;
    indexBufferPtr += 3;

    indexCount   += (triangles+1)*3;
    elementCount += triangles+1;
}

void BatchRenderer::x_drawQuad_internal(const vec4* vertices, const std::shared_ptr<Texture>& texture, float tilingFactor, const vec4& color)
{
    // if we would go outside the bounds of the buffers do the next batch
    if (indexBufferPtr+6 >= indexBufferEnd || vertexBufferPtr + 4*sizeof(QuadVertex) >= vertexBufferEnd)
        nextBatch();

    int textureIndex = 0;
    float layer = 0;
    if(texture)
    {
        layer = (float)texture->selectedLayer();
        textureIndex = addTexture(texture); // add new texture
    }

    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    auto bPtr = (QuadVertex*) vertexBufferPtr;
    for (size_t i = 0; i < 4; i++)
    {
        bPtr->position = vertices[i];
        bPtr->color = color;
        bPtr->texCoord = textureCoords[i];
        bPtr->texLayer = layer;
        bPtr->texIndex = (float)textureIndex;
        bPtr->tilingFactor = tilingFactor;
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

    indexCount   += 6;
    elementCount += 4;
}

