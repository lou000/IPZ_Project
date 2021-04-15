#pragma once
#include "shader.h"

class BufferElement{
    friend class BufferLayout;
    friend class VertexArray;

public:
    BufferElement(Shader::DataType type, const char* name, bool normalized = false)
        : name(name), type(type), normalized(normalized){}

private:
    const char* name;
    const Shader::DataType type;
    uint size = 0;
    uint offset = 0;
    bool normalized = false;
};



class BufferLayout{
    friend class VertexArray;
public:
    BufferLayout(std::initializer_list<BufferElement> elements)
        : elements(elements)
    {
        processElements();
    }

    uint stride(){return m_stride;}

private:
    uint m_stride = 0;
    std::vector<BufferElement> elements;

    void processElements();

};



class VertexBuffer{
    friend class VertexArray;

public:
    VertexBuffer(const BufferLayout& layout, uint size, void* data = nullptr);
    ~VertexBuffer();

    void bind();
    void unbind();
    void setData(const void* data, uint size);
    uint size(){return m_size;}
    BufferLayout layout(){return m_layout;}

private:
    uint id = 0;
    uint m_size = 0;
    BufferLayout m_layout;

};



class IndexBuffer{

public:
    IndexBuffer(uint* indices, uint count);
    ~IndexBuffer();

    void bind();
    void unbind();
    uint count(){return m_count;}

private:
    uint id = 0;
    uint m_count = 0;
};



class VertexArray{

public:
    VertexArray();
    ~VertexArray();

    void bind();
    void unbind();

    void addVBuffer(std::shared_ptr<VertexBuffer> buffer);
    void setIBuffer(std::shared_ptr<IndexBuffer> buffer);

    std::shared_ptr<IndexBuffer> indexBuffer(){return iBuffer;}
    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers(){return vBuffers;}

private:
    uint id = 0;
    uint vBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>> vBuffers;
    std::shared_ptr<IndexBuffer> iBuffer;

};

