#pragma once
#include <array>
#include "shader.h"
#include "camera.h"
#include "renderable.h"
#include <map>
#define MAX_VERTEX_BUFFER_SIZE 0xFFFFFF
#define MAX_INDEX_BUFFER_SIZE 0x30000 //this is max for uint16

struct QuadVertex{
    vec3 position;
    vec4 color;
    vec2 texCoord;
    float texIndex;
    float tilingFactor; //???
};

/* -----------------------------------------------------------------------
TODO: This will become implicit batch renderer for debug/2d rendering.
- enable switching between ortho and 3d camera
- hardcode the pipeline, maybe enable switching shaders and some other useful things, flush after every change
- create opengl vertex buffer when flushing? to enable switching buffer layout, or maybe make it fixed layout
- make all the debug drawing functions, draw line, draw quad, draw cube, !draw text(might be special shader becouse of distance field fonts?)!
- actually when i think about it, font rendering should be on separate buffer
----------------------------------------------------------------------- */
class ImRender{

    ImRender() = default;
    static ImRender& getInstance(){
        static ImRender instance;
        return instance;
    }

public:
    ImRender(ImRender const&)       = delete;
    void operator=(ImRender const&) = delete;

    static void init(){getInstance().x_init();}
    static void begin(const std::string& renderable){getInstance().x_begin(renderable);}
    static void end(){getInstance().x_end();}
    static void DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture= nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_DrawQuad(transform, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture = nullptr,
                  float tilingFactor = 1.f, const vec4 &tintColor = {1,1,1,1})
    {getInstance().x_DrawQuad(pos, size, texture, tilingFactor, tintColor);}
    static void DrawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor)
    {getInstance().x_DrawQuad(pos, size, tintColor);}

    static void DrawMesh(const mat4& translation, const mat4& rotation, const mat4& scale, const std::shared_ptr<MeshFile> &mesh, const vec4 &color)
    {getInstance().x_DrawMesh(translation, rotation, scale, mesh, color);}
    static void DrawMesh(const vec3 &pos, const vec3 &size, const std::shared_ptr<MeshFile>& mesh, const vec4& color)
    {getInstance().x_DrawMesh(pos, size, mesh, color);}


    static void setViewPort(uvec2 pos, uvec2 size){getInstance().x_setViewPort(pos, size);}
    static void setClearColor(vec4 color){getInstance().x_setClearColor(color);}
    static void setCamera(std::shared_ptr<Camera> camera){getInstance().x_setCamera(camera);}
    static std::shared_ptr<Camera> getCamera(){return getInstance().x_getCamera();}
    static void addRenderable(std::shared_ptr<Renderable> renderable){getInstance().x_addRenderable(renderable);}


private:
    std::shared_ptr<VertexArray> vertexArray = nullptr;

    byte* vertexBuffer    = nullptr;
    byte* vertexBufferPtr = nullptr;
    byte* vertexBufferEnd = nullptr;

    uint16* indexBuffer = nullptr;
    uint16* indexBufferPtr = nullptr;
    uint16* indexBufferEnd = nullptr;


    uint indexCount   = 0;
    uint elementCount = 0;

    std::map<std::string, std::shared_ptr<Renderable>> renderables;
    std::shared_ptr<Renderable> currentRenderable;
    std::shared_ptr<Camera> m_camera = nullptr;


    void x_init();
    void x_begin(const std::string& renderable);
    void x_end();

    void x_DrawQuad(const mat4 &transform, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_DrawQuad(const vec3 &pos, const vec2 &size, const std::shared_ptr<Texture> &texture,
                   float tilingFactor, const vec4 &tintColor);
    void x_DrawQuad(const vec3 &pos, const vec2 &size, const vec4 &tintColor);

    void x_DrawMesh(const vec3& pos, const vec3& size, const std::shared_ptr<MeshFile> &mesh, const vec4& color);
    void x_DrawMesh(const mat4& translation, const mat4& rotation, const mat4& scale, const std::shared_ptr<MeshFile> &mesh, const vec4 &color);



    void x_setViewPort(uvec2 pos, uvec2 size);
    void x_setClearColor(vec4 color);
    void x_setCamera(std::shared_ptr<Camera> camera){m_camera = camera;}
    std::shared_ptr<Camera> x_getCamera(){return m_camera;}
    void x_addRenderable(std::shared_ptr<Renderable> renderable);
    //TODO: removeRenderable

    void startBatch();
    void nextBatch();
    void flush();

};








