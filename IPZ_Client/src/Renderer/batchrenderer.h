#pragma once
#include <array>
#include <map>
#include "shader.h"
#include "camera.h"
#include "buffer.h"
#include "graphicscontext.h"
#define MAX_VERTEX_BUFFER_SIZE 0xFFFFFF
#define MAX_INDEX_BUFFER_SIZE 0xFFFE //this is max for uint16 which we use

/* -----------------------------------------------------------------------
TODO: This will become implicit batch renderer for debug/2d rendering.
- enable switching between ortho and 3d camera
- hardcode the pipeline, maybe enable switching shaders and some other useful things, flush after every change
- create opengl vertex buffer when flushing? to enable switching buffer layout, or maybe make it fixed layout
- make all the debug drawing functions, draw line, draw quad, draw cube, !draw text(might be special shader becouse of distance field fonts?)!
- actually when i think about it, font rendering should be on separate buffer
----------------------------------------------------------------------- */
class BatchRenderer{
    struct QuadVertex{
        vec4 position;
        vec4 color;
        vec2 texCoord;
        float texIndex;
        float tilingFactor;
    };

    BatchRenderer() = default;
    static BatchRenderer& getInstance(){
        static BatchRenderer instance;
        return instance;
    }

public:
    BatchRenderer(BatchRenderer const&)       = delete;
    void operator=(BatchRenderer const&) = delete;

    static void init(){getInstance().x_init();}
    static void begin(){getInstance().x_begin();}
    static void end(){getInstance().x_end();}
    static void drawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture= nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_drawQuad(transform, texture, tilingFactor, tintColor);}
    static void drawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture = nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_drawQuad(pos, size, texture, tilingFactor, tintColor);}
    static void drawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor)
    {getInstance().x_drawQuad(pos, size, tintColor);}
    static void drawQuad(const vec2 &pos, const vec2 &size, const vec4 &tintColor)
    {getInstance().x_drawQuad(pos, size, tintColor);}

    static void drawLine(const vec2& posStart, const vec2& posEnd, float width, const vec4& color)
    {getInstance().x_drawLine(posStart, posEnd, width, color);}
    static void drawLine(const vec3& posStart, const vec3& posEnd, float width, const vec4& color)
    {getInstance().x_drawLine(posStart, posEnd, width, color);}
    static void drawCircle(const vec2& pos, float radius, int triangles, const vec4& color)
    {getInstance().x_drawCircle(pos, radius, triangles, color);}


    static void setShader(std::shared_ptr<Shader> shader){getInstance().x_setShader(shader);}

private:
    mat4 viewProj3d;
    mat4 viewProjOrtho;

    std::shared_ptr<VertexArray> vertexArray = nullptr;

    int* texSamplers     = nullptr;
    byte* vertexBuffer    = nullptr;
    byte* vertexBufferPtr = nullptr;
    byte* vertexBufferEnd = nullptr;

    uint16* indexBuffer    = nullptr;
    uint16* indexBufferPtr = nullptr;
    uint16* indexBufferEnd = nullptr;

    uint indexCount      = 0;
    uint elementCount    = 0;

    int maxTextureSlots = 0;
    int textureCount    = 1;

    std::shared_ptr<Shader> m_currentShader = nullptr;
    std::vector<std::shared_ptr<Texture>> textureSlots;
    std::shared_ptr<Texture> whiteTex;

    void x_init();
    void x_begin();
    void x_end();


    void x_drawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_drawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_drawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor);
    void x_drawQuad(const vec2 &pos, const vec2 &size, const vec4 &tintColor);

    void x_drawLine(const vec2& posStart, const vec2& posEnd, float width, const vec4& color);
    void x_drawLine(const vec3& posStart, const vec3& posEnd, float width, const vec4& color);
    void x_drawCircle(const vec2& pos, float radius, int triangles, const vec4& color);

    void x_setShader(std::shared_ptr<Shader> shader){m_currentShader = shader;}

    int addTexture(const std::shared_ptr<Texture>& texture);
    void startBatch();
    void nextBatch();
    void flush();

    void x_drawQuad_internal(const vec4 *vertices, const std::shared_ptr<Texture> &texture, float tilingFactor, const vec4 &color);
};








