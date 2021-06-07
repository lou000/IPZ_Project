#pragma once
#include "../Core/utilities.h"
#include "../AssetManagement/assets.h"
#include "camera.h"
#include "buffer.h"
#include "graphicscontext.h"

using namespace glm;



// do instanced rendering, setup queu of meshes and models, render them at "end"
// https://learnopengl.com/Advanced-OpenGL/Instancing
// https://www.informit.com/articles/article.aspx?p=2033340&seqNum=5 normal matrix from model matrix!!

class MeshRenderer
{
    MeshRenderer() = default;
    static MeshRenderer& getInstance(){
        static MeshRenderer instance;
        return instance;
    }
public:
    MeshRenderer(MeshRenderer const&)   = delete;
    void operator=(MeshRenderer const&) = delete;

    static void init(){getInstance().x_init();}
    static void begin(){getInstance().x_begin();}
    static void end(){getInstance().x_end();}
    static void setShader(std::shared_ptr<Shader> shader){getInstance().x_setShader(shader);}

    static void drawMesh(const mat4& model, const std::shared_ptr<Mesh> &mesh, const vec4 &color = {1.f,1.f,1.f,1.f})
    {getInstance().x_drawMesh(model, mesh, color);}
    static void drawMesh(const vec3 &pos, const vec3 &size, const std::shared_ptr<Mesh>& mesh, const vec4& color = {1.f,1.f,1.f,1.f})
    {getInstance().x_drawMesh(pos, size, mesh, color);}

    static std::shared_ptr<Mesh> createCubeSphere(int vPerEdge);
    static std::shared_ptr<Mesh> createQuadMeshGrid(vec3* points, uint xSize, uint zSize, vec4* palette, uint pCount);
    static std::shared_ptr<Mesh> createSmoothMeshGrid(vec3* points, uint xSize, uint zSize, vec4* palette, uint pCount);

private:

    std::shared_ptr<Shader> currentShader = nullptr;

    void x_init();
    void x_begin();
    void x_end();
    void x_setShader(std::shared_ptr<Shader> shader) {currentShader = shader;}

    void x_drawMesh(const vec3& pos, const vec3& size, const std::shared_ptr<Mesh> &mesh, const vec4& color);
    void x_drawMesh(const mat4& model, const std::shared_ptr<Mesh> &mesh, const vec4 &color);
};

