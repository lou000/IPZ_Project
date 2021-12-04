#include "mesh.h"
#include "../Core/utilities.h"

Mesh::Mesh(float *vertexData, size_t vCount, uint16 *indexData, size_t iCount, bool textured)
    : textured(textured)
{
    ASSERT(vertexData && indexData);
    auto iBuffer = std::make_shared<IndexBuffer>(iCount*sizeof(uint16), indexData);

    if(textured)
    {
        BufferLayout layout = {
            {BufferElement::Float3, "a_Position" },
            {BufferElement::Float3, "a_Normal"   },
            {BufferElement::Float2, "a_TexCoords"}
        };

        auto vBuffer = {std::make_shared<VertexBuffer>(layout, vCount*sizeof(MeshVertex), vertexData)};
        m_vao = std::make_shared<VertexArray>(vBuffer, iBuffer);
    }
    else
    {
        BufferLayout layout = {
            {BufferElement::Float3, "a_Position" },
            {BufferElement::Float3, "a_Normal"   },
            {BufferElement::Float2, "a_TexCoords"},
            {BufferElement::Float4, "a_Color"}
        };

        auto vBuffer = {std::make_shared<VertexBuffer>(layout, vCount*sizeof(MeshVertexColored), vertexData)};
        m_vao = std::make_shared<VertexArray>(vBuffer, iBuffer);
    }
}
