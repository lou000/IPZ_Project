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

    static void init(){getInstance()._init();}
    static void begin(){getInstance()._begin();}
    static void end(){getInstance()._end();}
    static void DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture= nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance()._DrawQuad(transform, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture = nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance()._DrawQuad(pos, size, texture, tilingFactor, tintColor);}


private:
    static const uint maxVertices = 200;  // hmm lets see how this goes
    static const uint maxIndices  = 200;

    static const uint maxTexturesPerBuffer = 32; // idk about that
//    const uint maxTexturesTotal = 32*5; for now we make it single buffer

    QuadVertex* intermBuffer    = nullptr;
    QuadVertex* intermBufferPtr = nullptr;

    uint indexCount   = 0;
    uint textureCount = 0;

    std::shared_ptr<VertexArray>  vertexArray;
    std::shared_ptr<VertexBuffer> currentBuffer;

    std::shared_ptr<Texture> whiteTex;
    std::array<std::shared_ptr<Texture>, maxTexturesPerBuffer> textureSlots;

    int texSamplers[maxTexturesPerBuffer];

    void _init();
    void _begin();
    void _end();
    void _DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void _DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void startBatch();
    void nextBatch();
    void flush();


};








