#pragma once
#include "buffer.h"
#include <array>
#define MAX_TEXTURE_SLOTS 32


enum RenderSpecType{
    texturedQuad,
    mesh,
};

class RenderSpec
{
public:
    //In future might want more then one buffer per renderable?
    RenderSpec(const std::string& name, RenderSpecType type);

    RenderSpecType type(){return m_type;}
    const std::string& name() {return m_name;}
    std::shared_ptr<VertexBuffer> buffer(){return m_buffer;}
    std::shared_ptr<Shader> shader(){return m_shader;}

    void addVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);
    void setShader(std::shared_ptr<Shader> shader);

    virtual void onBegin() = 0;
    virtual void onFlush() = 0;

protected:
    const RenderSpecType m_type;
    const std::string m_name;
    std::shared_ptr<VertexBuffer> m_buffer;
    std::shared_ptr<Shader> m_shader;
};

class Renderable
{
    Renderable() = delete;

    virtual void render() = 0;

private:
    std::shared_ptr<RenderSpec> renderSpec;
};

class TexturedQuad : public RenderSpec
{
public:
    struct QuadVertex{
        vec3 position;
        vec4 color;
        vec2 texCoord;
        float texIndex;
        float tilingFactor; //???
    };
    TexturedQuad(const std::string& name, std::shared_ptr<Shader> shader, uint maxVBufferSize);

    void onBegin() override;
    void onFlush() override;
    int addTexture(std::shared_ptr<Texture> texture); //returns 0 if buffer is full otherwise returns texIndex

private:
    int texSamplers[MAX_TEXTURE_SLOTS];
    std::array<std::shared_ptr<Texture>, MAX_TEXTURE_SLOTS> textureSlots;
    std::shared_ptr<Texture> whiteTex;
    uint textureCount = 1;

};

class ColoredMesh : public RenderSpec
{
public:
    struct MeshVertex{
        vec3 position;
        vec3 normals;
    };
    ColoredMesh(const std::string& name, std::shared_ptr<Shader> shader, uint maxVBufferSize);
    virtual void onBegin() override;
    virtual void onFlush() override;
    void setColor(vec4 color){m_color = color;}
    vec4 color(){return m_color;}

private:
    vec4 m_color = {0,0,0,0};

};
