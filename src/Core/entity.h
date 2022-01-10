#pragma once
#include "math.h"
#include "../AssetManagement/assets.h"

using namespace glm;
class Entity
{
    friend class Scene;

public:
    Entity(){};
    Entity(vec3 pos, std::shared_ptr<Model> model);
    mat4 getModelMatrix();
    void setOverrideColor(vec4 color);


    enum Type{
        Base,
        PointLight
    };

    Type type = Base;
    vec4 color = {0,0,0,0};
    vec3 pos = {0, 0, 0};
    vec3 scale = {1, 1, 1};
    quat rotation = {1,0,0,0};;
    std::shared_ptr<Model> model;
    bool renderable = false;
    bool enabled() const;
protected:
    bool m_enabled = false;

};

