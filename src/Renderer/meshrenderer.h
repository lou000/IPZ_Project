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

    static std::shared_ptr<Mesh> createCubeSphere(int vPerEdge);
    static std::shared_ptr<Mesh> createQuadMeshGrid(vec3 *points, uint xSize, uint zSize);
    static std::shared_ptr<Mesh> createSmoothMeshGrid(vec4 *points, uint xSize, uint zSize);
    static std::shared_ptr<Model> createTriMeshGrid(const std::string &name, uint xSize, uint zSize);
};

