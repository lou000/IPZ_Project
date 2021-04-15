#include "renderable.h"
//Well there is not much going on here but im sure something will come up

Renderable::Renderable(const std::string &name, RenderableType type) : m_type(type), m_name(name)
{

}

void Renderable::addVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
    m_buffer = vertexBuffer;
}

void Renderable::setShader(std::shared_ptr<Shader> shader)
{
    m_shader = shader;
}




////////////////////////////////////////////////////////////////////////////////////////
TexturedQuad::TexturedQuad(const std::string &name, std::shared_ptr<Shader> shader, uint maxVBufferSize)
    : Renderable(name, RenderableType::texturedQuad)
{
    BufferLayout layout = {
        {Shader::Float3, "a_Position"    },
        {Shader::Float4, "a_Color"       },
        {Shader::Float2, "a_TexCoord"    },
        {Shader::Float , "a_TexIndex"    },
        {Shader::Float , "a_TilingFactor"}
    };
    m_buffer = std::make_shared<VertexBuffer>(layout, maxVBufferSize);
    m_shader = shader;

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

int TexturedQuad::addTexture(std::shared_ptr<Texture> texture)
{
    for (uint32_t i = 1; i < textureCount; i++)
    {
        if (textureSlots[i]->id() == texture->id())
            return i;
    }

    if (textureCount >= textureSlots.size())
        return 0;

    int textureIndex = textureCount;
    textureSlots[textureCount] = texture;
    textureCount++;
    return textureIndex;
}



////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh(const std::string &name, std::shared_ptr<Shader> shader, uint maxVBufferSize)
    :Renderable(name, RenderableType::mesh)
{
    BufferLayout layout = {
        {Shader::Float3, "a_Position"},
        {Shader::Float3, "a_Normal"  }
    };
    m_buffer = std::make_shared<VertexBuffer>(layout, maxVBufferSize);
    m_shader = shader;

}

void Mesh::onBegin()
{
    m_shader->bind();
}

void Mesh::onFlush()
{
    //Nothing here... yet
    return;
}
