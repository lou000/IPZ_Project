#pragma once
#include "math.h"
#include "../AssetManagement/assets.h"

using namespace glm;
class Entity
{
    friend class Scene;
    friend class Serializer;

public:
    mat4 getModelMatrix();
    void setOverrideColor(vec4 color);
    bool enabled() const;

    enum Type{
        Base,
        PointLight,
        C4Board,
        C4Puck,
        Decoration
    };
    Type getType(){return m_type;}

    //       SERIALIZED      //
    //-----------------------//
    vec4 color =    {0,0,0,0};
    vec3 pos =      {0, 0, 0};
    vec3 scale =    {1, 1, 1};
    quat rotation = {1,0,0,0};
    std::shared_ptr<Model> model; // serialize mesh names, count on asset manager
    bool renderable = false;
    //-----------------------//

protected:
    Entity(Entity::Type type) : m_type(type){}
    Type m_type =     Base;
    bool m_enabled = true;

private:
    uint64 m_id = 0; //SERIALIZED
};

