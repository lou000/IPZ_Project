#pragma once


// For now this is base class that represents the "game"
class Scene
{
public:
    Scene(){};
    virtual ~Scene(){};
    virtual void onUpdate(float dt) = 0;
};

