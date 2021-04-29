#pragma once
#include <array>
#include <map>
#include "shader.h"
#include "camera.h"
#include "buffer.h"
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
        vec3 position;
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
    static void DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture= nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_DrawQuad(transform, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture = nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_DrawQuad(pos, size, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor)
    {getInstance().x_DrawQuad(pos, size, tintColor);}

    static void setCamera(std::shared_ptr<Camera> camera){getInstance().x_setCamera(camera);}
    static std::shared_ptr<Camera> getCamera(){return getInstance().x_getCamera();}
    static void setShader(std::shared_ptr<Shader> shader){getInstance().x_setShader(shader);}

private:
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

    std::shared_ptr<Camera> m_camera        = nullptr;
    std::shared_ptr<Shader> m_currentShader = nullptr;
    std::vector<std::shared_ptr<Texture>> textureSlots;
    std::shared_ptr<Texture> whiteTex;

    void x_init();
    void x_begin();
    void x_end();


    void x_DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_DrawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor);


    void x_setShader(std::shared_ptr<Shader> shader){m_currentShader = shader;}
    void x_setCamera(std::shared_ptr<Camera> camera){m_camera = camera;}
    std::shared_ptr<Camera> x_getCamera(){return m_camera;}

    int addTexture(const std::shared_ptr<Texture>& texture);
    void startBatch();
    void nextBatch();
    void flush();

};








