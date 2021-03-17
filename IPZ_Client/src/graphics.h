#define PI 3.14159265358979323846
#include "olcPixelGameEngine.h"
#include "olcPGEX_Graphics3D.h"
#define ASSERT_NOT_REACHED _ASSERT_EXPR(true, "ASSERT_NOT_REACHED");

namespace IPZ {
typedef olc::vf2d vec2;
typedef olc::GFX3D::vec3d vec3;
typedef olc::GFX3D::mat4x4 mat4;


std::ostream& operator<<(std::ostream& os, const vec3& vec)
{
    os <<"{"<< vec.x << "," << vec.y << "," << vec.z<<"}";
    return os;
}

struct Camera{
    vec3 pos = {0,0,-2};
    vec3 dir = {0,0,1};
    vec2 screenSize = {0,0};
    float fov = 90.0f;
    float clipNear = 0.1f;
    float clipFar  = 1000.0f;
};


enum Face{
    North,
    South,
    East,
    West,
    Top,
    Bottom
};

struct Sprite{
    olc::Sprite* sprite = nullptr;
    olc::Decal*  decal  = nullptr;
    vec3 pos[4];

    Sprite()
    {
        sprite = new olc::Sprite();
        decal  = new olc::Decal(sprite);
    }
    Sprite(const std::string& file)
    {
        sprite = new olc::Sprite(file);

        decal  = new olc::Decal(sprite);
    }
    ~Sprite()
    {
        delete sprite;
        delete decal;
    }

    int32_t width()
    {
        return sprite->width;
    }
    int32_t height()
    {
        return sprite->height;
    }
};



}



