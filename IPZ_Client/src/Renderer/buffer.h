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
    IndexBuffer(uint size, uint16* indices = nullptr);
    ~IndexBuffer();

    void bind();
    void unbind();
    void setData(const uint16* data, uint size);
    uint count(){return m_count;}

private:
    uint id = 0;
    uint m_count = 0;
};



class VertexArray{

public:
    VertexArray(std::initializer_list<std::shared_ptr<VertexBuffer>> vBuffers,
                std::shared_ptr<IndexBuffer> iBuffer);
    VertexArray();
    ~VertexArray();

    void bind();
    void unbind();

    void addVBuffer(std::shared_ptr<VertexBuffer> buffer);
    void setIBuffer(std::shared_ptr<IndexBuffer> buffer);
    void clearVBuffers();
    //TODO: removeVBuffer?

    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers(){return vBuffers;}
    std::shared_ptr<IndexBuffer> indexBuffer(){return iBuffer;}

private:
    uint id = 0;
    std::vector<std::shared_ptr<VertexBuffer>> vBuffers;
    std::shared_ptr<IndexBuffer> iBuffer;
    uint vBufferIndex = 0;

};

struct FrameBufferAttachment
{
    GLenum format = 0;
    GLenum type = 0;
    bool renderBuffer = true;
};

class FrameBuffer
{
    //TODO: fill out the functions, and remember to blit to screen for now
public:
    FrameBuffer(uint width, uint height, std::vector<FrameBufferAttachment> colorAtachments,
                FrameBufferAttachment depthAttachment = {}, uint samples = 1);
    void resize(uint width, uint height);
    void bind();
    void bind(std::vector<GLenum> attachments);
    void unbind();
    void blitToFrontBuffer();

private:
    void update();

    uint id      = 0;
    uint width   = 0;
    uint height  = 0;
    uint samples = 1;
    int maxAttachments = 0;

    std::vector<FrameBufferAttachment> colorAttachments;
    FrameBufferAttachment depthAttachment = {};

    std::vector<uint> rboIds;

    std::vector<std::shared_ptr<Texture>> attachedTextures; //give weak pointer if we take from here
    std::shared_ptr<Texture> depthTexture = nullptr; //could be depth+stencil
};
