#include "renderer.h"
#include "asset_manager.h"

extern "C" {    // this should help select dedicated gpu?
_declspec(dllexport) DWORD NvOptimusEnablement = 1;
_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void Renderer::x_init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    intermBuffer = new QuadVertex[maxVertices];
    vertexArray = std::make_shared<VertexArray>();
    BufferLayout layout = {
        {Shader::Float3, "a_Position"    },
        {Shader::Float4, "a_Color"       },
        {Shader::Float2, "a_TexCoord"    },
        {Shader::Float , "a_TexIndex"    },
        {Shader::Float , "a_TilingFactor"}
    };
    currentBuffer = std::make_shared<VertexBuffer>(layout, maxVertices*(uint)sizeof(QuadVertex), intermBuffer);
    vertexArray->addVBuffer(currentBuffer);
    auto shader = AssetManager::getShader("test");
    currentBuffer->setShader(shader);
    uint* indices = new uint[maxIndices];

    uint offset = 0;

    for(uint i=0; i<maxIndices-6; i+=6, offset+=4)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;

        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
    }

    auto indexBuffer = std::make_shared<IndexBuffer>(indices, maxIndices);
    vertexArray->setIBuffer(indexBuffer);
    delete[] indices;

    for(uint i=0; i<maxTexturesPerBuffer; i++)
        texSamplers[i] = i;


    whiteTex = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTex->setTextureData(&whiteData, sizeof(uint));
    textureSlots[0] = whiteTex;

    shader->bind();
}

void Renderer::x_begin()
{
    glClear(GL_COLOR_BUFFER_BIT);
    //Need camera class
//    UNUSED(camera);
//    UNUSED(transform);
    mat4 viewProj = ortho(-2.0f,2.0f,-2.0f,2.0f,0.0f,100.0f);

    currentBuffer->shader()->bind();
    currentBuffer->shader()->setUniform("u_ViewProjection", Shader::Mat4, viewProj);
    currentBuffer->shader()->setUniformArray("u_Textures", Shader::Int, texSamplers, maxTexturesPerBuffer);

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
    textureCount = 1;
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
    currentBuffer->setData(intermBuffer, size);

    for (uint32_t i = 0; i < textureCount; i++)
        textureSlots[i]->bind(i);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::x_DrawQuad(const vec3& pos, const vec2& size, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{

    mat4 transform = translate(mat4(1.0f), pos)
                     * scale(mat4(1.0f), {size.x, size.y, 1.0f});
    x_DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer::x_setViewPort(uvec2 pos, uvec2 size)
{
    glViewport(pos.x, pos.y, size.x, size.y);
}

void Renderer::x_setClearColor(vec4 color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::x_DrawQuad(const mat4& transform, const std::shared_ptr<Texture>& texture,
                         float tilingFactor, const vec4& tintColor)
{
    constexpr vec2 textureCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    if (indexCount + 6 >= maxIndices)
        nextBatch();

    int textureIndex = 0;
    if(texture == nullptr)
    {
        textureIndex = 0;
    }
    else
    {
        for (uint32_t i = 1; i < textureCount; i++)
        {
            if (textureSlots[i]->id() == texture->id())
            {
                textureIndex = i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            if (textureCount >= maxTexturesPerBuffer)
                nextBatch();

            textureIndex = textureCount;
            textureSlots[textureCount] = texture;
            textureCount++;
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        intermBufferPtr->position = transform * quadVertexPos[i];
        intermBufferPtr->color = tintColor;
        intermBufferPtr->texCoord = textureCoords[i];
        intermBufferPtr->texIndex = (float)textureIndex;
        intermBufferPtr->tilingFactor = tilingFactor;
        intermBufferPtr++;
    }

    indexCount += 6;
}

