#pragma once
#include "math.h"
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
    //NOTE: We might add this as a boolean check but you shouldn't set override color to
    //      invisible, instead you should set renderable to false
    vec4 overrideColor = {0,0,0,0};
    vec3 pos = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    quat rotation = {1,0,0,0};;
    std::shared_ptr<Model> model;
};

