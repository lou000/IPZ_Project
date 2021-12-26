#pragma once
#include "utilities.h"
#include "../AssetManagement/assets.h"

using namespace glm;
struct Entity
{
    Entity(){};
    Entity(vec3 pos, std::shared_ptr<Model> model);
    mat4 getModelMatrix();
    void setOverrideColor(vec4 color);
    void setModel(std::shared_ptr<Model> model);

    bool enabled = false;
    bool renderable = false;
    vec4 overrideColor = {1,1,1,1};
    vec3 pos = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    quat rotation = {1,0,0,0};;
    std::shared_ptr<Model> model;
};

