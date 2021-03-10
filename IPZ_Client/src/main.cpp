#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION
#define PI 3.14159265358979323846
#include "olcPixelGameEngine.h"

using namespace olc;

class Example : public olc::PixelGameEngine
{
public:
    olc::Sprite test;
    olc::Decal* testDecal;
    vf2d pos = vf2d(0,50);
    Example()
    {
        sAppName = "Unnamed Game Client";
    }

public:
    bool OnUserCreate() override
    {
        test.LoadFromFile("../assets/img/test.png");
        testDecal = new Decal(&test);
        return true;
    }

    bool OnUserUpdate(float dt) override
    {
        Clear(VERY_DARK_GREY);
        auto screenW = ScreenWidth();
        auto screenH = ScreenHeight();
        float scale = 0.5;
        float speed = 100;
        auto startingPos = -test.width/2 * scale;

        pos.x += speed * dt;
        pos.y = (screenH/2 - test.height*scale/2) * cos((pos.x-startingPos)*(2 * PI/(screenW-test.width*scale))) + screenH/2;
        if(pos.x > screenW + test.width*scale/2)
            pos.x = startingPos;

        vf2d centered = vf2d(pos.x - test.width*scale/2, pos.y - test.height*scale/2);
        DrawDecal(centered, testDecal, vf2d(scale, scale));
        return true;
    }
};


int main()
{
    Example demo;
    if (demo.Construct(640, 360, 2, 2, false, true))
        demo.Start();

    return 0;
}
