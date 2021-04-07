#pragma once
#include <array>
#include "shader.h"
#include "camera.h"
#include "buffer.h"

static const vec4 quadVertexPos[4] =
{
    { -0.5f, -0.5f, 0.0f, 1.0f },
    {  0.5f, -0.5f, 0.0f, 1.0f },
    {  0.5f,  0.5f, 0.0f, 1.0f },
    { -0.5f,  0.5f, 0.0f, 1.0f }
};

struct QuadVertex{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    float texIndex;
    float tilingFactor; //???
};

class Renderer{

    Renderer() = default;
    static Renderer& getInstance(){
        static Renderer instance;
        return instance;
    }

public:
    Renderer(Renderer const&)       = delete;
    void operator=(Renderer const&) = delete;

    static void init(){getInstance().x_init();}
    static void begin(){getInstance().x_begin();}
    static void end(){getInstance().x_end();}
    static void DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture= nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_DrawQuad(transform, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture = nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_DrawQuad(pos, size, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor)
    {getInstance().x_DrawQuad(pos, size, tintColor);}
    static void setViewPort(uvec2 pos, uvec2 size){getInstance().x_setViewPort(pos, size);}
    static void setClearColor(vec4 color){getInstance().x_setClearColor(color);}


private:
    static const uint maxVertices = 0xFFFFF;  // hmm lets see how this goes
    static const uint maxIndices  = 0xFFFFF;

    static const uint maxTexturesPerBuffer = 32; // idk about that
//    const uint maxTexturesTotal = 32*5; for now we make it single buffer

    QuadVertex* intermBuffer    = nullptr;
    QuadVertex* intermBufferPtr = nullptr;

    uint indexCount   = 0;
    uint textureCount = 0;

    std::shared_ptr<VertexArray>  vertexArray;
    std::shared_ptr<VertexBuffer> currentBuffer;
    std::shared_ptr<Camera> camera = nullptr;

    std::shared_ptr<Texture> whiteTex;
    std::array<std::shared_ptr<Texture>, maxTexturesPerBuffer> textureSlots;

    int texSamplers[maxTexturesPerBuffer];

    void x_init();
    void x_begin();
    void x_end();
    void x_DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_DrawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor);
    void x_setViewPort(uvec2 pos, uvec2 size);
    void x_setClearColor(vec4 color);
    void x_setCamera(std::shared_ptr<Camera> camera);
    void x_getCamera(std::shared_ptr<Camera> camera);

    void startBatch();
    void nextBatch();
    void flush();

};








