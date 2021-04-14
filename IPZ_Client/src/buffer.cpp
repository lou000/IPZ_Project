#include "buffer.h"

void BufferLayout::processElements()
{
    uint offset = 0;
    for(auto& element : elements)
    {
        element.size = Shader::typeComponentSize(element.type);
        element.offset = offset;
        offset += element.size;
        m_stride += element.size;
    }
}

VertexBuffer::VertexBuffer(const BufferLayout &layout, uint size, void *data)
    : m_size(size), m_layout(layout)
{
    glCreateBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    if(data)
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &id);
}

void VertexBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VertexBuffer::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setData(const void *data, uint size)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

IndexBuffer::IndexBuffer(uint *indices, uint count)
{
    glCreateBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &id);
}

void IndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IndexBuffer::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}




VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArray::bind()
{
    glBindVertexArray(id);
}

void VertexArray::unbind()
{
    glBindVertexArray(0);
}

void VertexArray::addVBuffer(std::shared_ptr<VertexBuffer> buffer)
{
    glBindVertexArray(id);
    buffer->bind();
    for(auto& element : buffer->m_layout.elements)
    {
        switch (element.type) {

        case Shader::Int:
        {
            glEnableVertexAttribArray(vBufferIndex);
            glVertexAttribIPointer(vBufferIndex, Shader::typeComponentCount(element.type), Shader::typeToNative(element.type),
                                   buffer->m_layout.m_stride, (const void*)(size_t)element.offset);

            vBufferIndex++;
            break;
        }
        case Shader::Float:
        case Shader::Float2:
        case Shader::Float3:
        case Shader::Float4:
        {
            glEnableVertexAttribArray(vBufferIndex);
            glVertexAttribPointer(vBufferIndex, Shader::typeComponentCount(element.type),Shader::typeToNative(element.type),
                                  element.normalized, buffer->m_layout.m_stride, (const void*)(size_t)element.offset);

            vBufferIndex++;
            break;
        }
        case Shader::Mat3:
        case Shader::Mat4:
        {
            int count = Shader::typeComponentCount(element.type);
            for(int i=0; i<count; i++)
            {
                glEnableVertexAttribArray(vBufferIndex);
                glVertexAttribPointer(vBufferIndex, count, Shader::typeToNative(element.type),
                                      element.normalized, buffer->m_layout.m_stride, (const void*)(element.offset + sizeof(float) * count * i));
                glVertexAttribDivisor(vBufferIndex, 1);
                vBufferIndex++;
            }
            break;
        }
        }
    }
    vBuffers.push_back(buffer);
}

void VertexArray::setIBuffer(std::shared_ptr<IndexBuffer> buffer)
{
    glBindVertexArray(id);
    buffer->bind();
    iBuffer = buffer;
}



