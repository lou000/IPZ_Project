#pragma once
#include "math.h"
#include "../AssetManagement/assets.h"

using namespace glm;
class Entity
{
    friend class Scene;
    friend class Serializer;

public:
    Entity(vec3 pos, std::shared_ptr<Model> model);
    mat4 getModelMatrix();
    void setOverrideColor(vec4 color);
    bool enabled() const;


    enum Type{
        Base,
        PointLight
    };

    //       SERIALIZED      //
    //-----------------------//
    Type type =     Base;
    vec4 color =    {0,0,0,0};
    vec3 pos =      {0, 0, 0};
    vec3 scale =    {1, 1, 1};
    quat rotation = {1,0,0,0};
    std::shared_ptr<Model> model; // serialize mesh names, count on asset manager
    bool renderable = false;
    //-----------------------//

protected:
    bool m_enabled = false;
private:
    Entity(uint64 id){}
    uint64 id = 0; //SERIALIZED
};

