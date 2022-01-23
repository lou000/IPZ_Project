#include "buffer.h"
#include "../Core/application.h"

void BufferLayout::appendElement(BufferElement element)
{
    element.size = BufferElement::typeComponentSize(element.type);
    elements.push_back(element);
    element.offset = m_stride;
    m_stride += element.size;
}

void BufferLayout::processElements()
{
    uint offset = 0;
    for(auto& element : elements)
    {
        element.size = BufferElement::typeComponentSize(element.type);
        element.offset = offset;
        offset += element.size;
        m_stride += element.size;
    }
}

VertexBuffer::VertexBuffer(const BufferLayout &layout, size_t size, void *data)
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

void VertexBuffer::setData(const void *data, size_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

IndexBuffer::IndexBuffer(size_t size, uint32 *indices)
    :m_size(size)
{
    glCreateBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    if(indices)
        glBufferData(GL_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
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

void IndexBuffer::setData(const uint32 *data, uint size)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}


VertexArray::VertexArray(std::initializer_list<std::shared_ptr<VertexBuffer>> vBuffers,
                         std::shared_ptr<IndexBuffer> iBuffer)
{
    glCreateVertexArrays(1, &id);
    for(auto& buffer : vBuffers)
        addVBuffer(buffer);
    setIBuffer(iBuffer);
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

        case BufferElement::Int:
        {
            glEnableVertexAttribArray(vBufferIndex);
            glVertexAttribIPointer(vBufferIndex, BufferElement::typeComponentCount(element.type), BufferElement::typeToNative(element.type),
                                   buffer->m_layout.m_stride, (const void*)(size_t)element.offset);

            vBufferIndex++;
            break;
        }
        case BufferElement::Float:
        case BufferElement::Float2:
        case BufferElement::Float3:
        case BufferElement::Float4:
        {
            glEnableVertexAttribArray(vBufferIndex);
            glVertexAttribPointer(vBufferIndex, BufferElement::typeComponentCount(element.type),BufferElement::typeToNative(element.type),
                                  element.normalized, buffer->m_layout.m_stride, (const void*)(size_t)element.offset);

            vBufferIndex++;
            break;
        }
        case BufferElement::Mat3:
        case BufferElement::Mat4:
        {
            int count = BufferElement::typeComponentCount(element.type);
            for(int i=0; i<count; i++)
            {
                glEnableVertexAttribArray(vBufferIndex);
                glVertexAttribPointer(vBufferIndex, count, BufferElement::typeToNative(element.type),
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

void VertexArray::popVBuffer()
{
    // well i hope it works
    auto buffer = vBuffers.back();
    buffer->bind();
    for(auto element : buffer->m_layout.elements)
    {
        UNUSED(element);
        glDisableVertexAttribArray(vBufferIndex);
        vBufferIndex--;
    }
    vBuffers.erase(vBuffers.end()-1);
}

void VertexArray::clearVBuffers()
{
    vBufferIndex = 0;
    vBuffers.clear();
}

void VertexArray::setIBuffer(std::shared_ptr<IndexBuffer> buffer)
{
    glBindVertexArray(id);
    buffer->bind();
    iBuffer = buffer;
}

FrameBuffer::FrameBuffer(uint width, uint height, uint depth, std::vector<FrameBufferAttachment> colorAttachments,
                         FrameBufferAttachment depthAttachment, uint samples)
    :width(width), height(height), depth(depth), samples(samples),
    colorAttachments(colorAttachments), depthAttachment(depthAttachment)
{
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments); // TODO: Move this somewhere where its not called everytime
    ASSERT((GLint)colorAttachments.size()<=maxAttachments);
    ASSERT(width>0 && height>0 && depth>0);
    int maxSamples = 0;
    glGetIntegerv (GL_MAX_SAMPLES, &maxSamples); // TODO: Move this somewhere where its not called everytime
    if(samples>(uint)maxSamples)
    {
        this->samples = maxSamples;
        WARN("OpenGL: Framebuffer samples exceed GL_MAX_SAMPLES, setting to max value of %d.", maxSamples);
    }
    update();
}

std::shared_ptr<Texture> FrameBuffer::getTexture(uint index)
{
    ASSERT(index<attachedTextures.size());
    return attachedTextures.at(index);
}

std::shared_ptr<Texture> FrameBuffer::getDepthTex()
{
    ASSERT(depthTexture);
    return depthTexture;
}

void FrameBuffer::resize(uint width, uint height, uint depth)
{
    if(width>0 && height>0 && depth>0)
    {
        this->width = width;
        this->height = height;
        this->depth = depth;
        update();
    }
}

void FrameBuffer::bind()    //binds all attachments
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glViewport(0, 0, width, height);
    if(colorAttachments.size() == 0)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else if(colorAttachments.size() == 1)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    }
    else
    {
        uint bufferCount = colorAttachments.size();
        std::vector<GLenum> buffers;
        buffers.resize(bufferCount);
        for(size_t i=0; i<bufferCount; i++)
        {
            buffers[i] = colorAttachments.at(i).type;
        }
        glDrawBuffers(bufferCount, buffers.data());
    }
}

void FrameBuffer::bindColorAttachment(uint index) //binds specific color attachment
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    auto colorBuffer = colorAttachments.at(index).type;
    glDrawBuffer(colorBuffer);
}

void FrameBuffer::bindDepthAttachment() // bind depth attachment
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::blitToFrontBuffer()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0,0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    unbind();
}

void FrameBuffer::clear(vec3 color)
{
    bind();
    glClearColor(color.r, color.g, color.b, 1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    unbind();
}

void FrameBuffer::clearColorAttachment(uint index, vec3 color)
{
    bindColorAttachment(index);
    glClearColor(color.r, color.g, color.b, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    unbind();
}

void FrameBuffer::clearDepthAttachment()
{
    bindDepthAttachment();
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    unbind();
}

void FrameBuffer::update()  // create/recreate framebuffer
{
    uint texCount = (uint)colorAttachments.size();
    if(id)  // delete all data if framebuffer exists
    {
        glDeleteFramebuffers(1, &id);
        depthTexture = nullptr;   //should auto delete from texture destructors
        attachedTextures.clear();
        glDeleteRenderbuffers(rboIds.size(), rboIds.data());
    }
    glCreateFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    if(texCount>0)
    {
        for (size_t i=0; i<texCount; i++)
        {
            auto att = colorAttachments[i];
            ASSERT(!(att.renderBuffer && depth>1), "Framebuffer: Renderbuffer cant be 3D");
            ASSERT(att.type >= GL_COLOR_ATTACHMENT0 && att.type <= (GL_COLOR_ATTACHMENT0+(uint)maxAttachments-1));
            if(att.renderBuffer)
            {
                uint rbo;
                glGenRenderbuffers(1, &rbo);
                rboIds.push_back(rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                if(samples>1)
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, att.format, width, height);
                else
                    glRenderbufferStorage(GL_RENDERBUFFER, att.format, width, height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, att.type, GL_RENDERBUFFER, rbo);
            }
            else
            {
                auto texture = std::make_shared<Texture>(width, height, depth, att.format, GL_CLAMP_TO_EDGE, samples);
                attachedTextures.push_back(texture);
                if(depth>1)
                    glFramebufferTexture(GL_FRAMEBUFFER, att.type, texture->id(), 0);
                else
                    glFramebufferTexture2D(GL_FRAMEBUFFER, att.type,
                                           samples>1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texture->id(), 0);
            }
        }
    }
    if(depthAttachment.format>0) //if initialized
    {
        ASSERT(!(depthAttachment.renderBuffer && depth>1), "Framebuffer: Renderbuffer cant be 3D");
        if(depthAttachment.renderBuffer)
        {
            uint rbo;
            glGenRenderbuffers(1, &rbo);
            rboIds.push_back(rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            if(samples>1)
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthAttachment.format, width, height);
            else
                glRenderbufferStorage(GL_RENDERBUFFER, depthAttachment.format, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, depthAttachment.type, GL_RENDERBUFFER, rbo);
        }
        else
        {
            ASSERT(depthAttachment.type == GL_DEPTH_ATTACHMENT || depthAttachment.type == GL_DEPTH_STENCIL_ATTACHMENT);
            auto texture = std::make_shared<Texture>(width, height, depth, depthAttachment.format, GL_CLAMP_TO_EDGE, samples);
            depthTexture = texture;
            if(depth>1)
                glFramebufferTexture(GL_FRAMEBUFFER, depthAttachment.type, texture->id(), 0);
            else
                glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachment.type,
                                       samples>1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texture->id(), 0);
        }
    }
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "OpenGL: Framebuffer is incomplete!\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

StorageBuffer::StorageBuffer(size_t size, void *data, uint usage)
    :size(size)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void StorageBuffer::bind(uint bufferIndex)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferIndex, id);
}

void StorageBuffer::unbind()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void StorageBuffer::setData(const void *data, size_t size)
{
    ASSERT(size<=this->size);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void StorageBuffer::setSubData(const void *data, size_t offset, size_t size)
{
    ASSERT(size+offset<=this->size);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}
