#pragma once
#include "utilities.h"
#include "../Renderer/mesh.h"

using namespace glm;
struct Entity
{
    Entity(){};
    Entity(vec3 pos, std::vector<std::shared_ptr<Mesh>> meshes);
    mat4 getModel();
    void setOverrideColor(vec4 color);
    void setMesh(std::shared_ptr<Mesh> mesh);

    bool enabled = false;
    bool renderable = false;
    vec4 overrideColor = {1,1,1,1};
    vec3 pos = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    quat rotation = {1,0,0,0};;
    std::vector<std::shared_ptr<Mesh>> meshes;
};

