#include "renderable.h"
//Well there is not much going on here but im sure something will come up

Renderable::Renderable(const std::string &name, RenderableType type) : m_type(type), m_name(name)
{
    m_vertexArray = std::make_shared<VertexArray>();
}

void Renderable::addVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
    m_buffer = vertexBuffer;
}

void Renderable::setIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
    m_indexBuffer = indexBuffer;
    m_vertexArray->setIBuffer(indexBuffer);
}

void Renderable::setShader(std::shared_ptr<Shader> shader)
{
    m_shader = shader;
}

TexturedQuad::TexturedQuad(const std::string &name, std::shared_ptr<Shader> shader, uint maxVertices)
    : Renderable(name, RenderableType::texturedQuad)
{
    BufferLayout layout = {
        {Shader::Float3, "a_Position"    },
        {Shader::Float4, "a_Color"       },
        {Shader::Float2, "a_TexCoord"    },
        {Shader::Float , "a_TexIndex"    },
        {Shader::Float , "a_TilingFactor"}
    };
    m_buffer = std::make_shared<VertexBuffer>(layout, maxVertices*(uint)sizeof(QuadVertex));
    m_vertexArray->addVBuffer(m_buffer);
    m_shader = shader;

    //this is wrong fix!
    maxIndices = (m_buffer->size()/m_buffer->layout().stride());
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
    m_indexBuffer = std::make_shared<IndexBuffer>(indices, maxIndices);
    m_vertexArray->setIBuffer(m_indexBuffer);
    delete[] indices;

    for(uint i=0; i<MAX_TEXTURE_SLOTS; i++)
        texSamplers[i] = i;

    whiteTex = std::make_shared<Texture>(1, 1);
    uint whiteData = 0xffffffff;
    whiteTex->setTextureData(&whiteData, sizeof(uint));
    textureSlots[0] = whiteTex;

}

void TexturedQuad::onBegin()
{
    m_shader->bind();
    m_shader->setUniformArray("u_Textures", Shader::Int, texSamplers, MAX_TEXTURE_SLOTS);

}

void TexturedQuad::onFlush()
{
    for (uint32_t i = 0; i < textureSlots.size(); i++)
        if(textureSlots[i])
            textureSlots[i]->bind(i);
    textureCount = 1;
}
