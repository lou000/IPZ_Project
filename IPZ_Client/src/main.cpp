#include "graphics.h"



using namespace IPZ;

class Example : public olc::PixelGameEngine
{
public:
    Sprite* sprite;
    Camera camera;
    olc::vf2d pos = olc::vf2d(0,50);
    Example()
    {
        sAppName = "Unnamed Game Client";
    }

public:
    bool OnUserCreate() override
    {
        sprite = new Sprite("../assets/img/test.png");
        return true;
    }

    bool OnUserUpdate(float dt) override
    {
        // WS keys to tilt camera
        if (GetKey(olc::Key::W).bHeld) camera.pitch += 1.0f * dt;
        if (GetKey(olc::Key::S).bHeld) camera.pitch -= 1.0f * dt;

        // DA Keys to manually rotate camera
        if (GetKey(olc::Key::D).bHeld) camera.angle += 1.0f * dt;
        if (GetKey(olc::Key::A).bHeld) camera.angle -= 1.0f * dt;

        // QZ Keys to zoom in or out
        if (GetKey(olc::Key::Q).bHeld) camera.zoom += 5.0f * dt;
        if (GetKey(olc::Key::Z).bHeld) camera.zoom -= 5.0f * dt;

        //pos
        if (GetKey(olc::Key::LEFT).bHeld) camera.pos.x   -= 1000.0f * dt / camera.zoom;
        if (GetKey(olc::Key::RIGHT).bHeld) camera.pos.x  += 1000.0f * dt / camera.zoom;
        if (GetKey(olc::Key::UP).bHeld) camera.pos.y     -= 1000.0f * dt / camera.zoom;
        if (GetKey(olc::Key::DOWN).bHeld) camera.pos.y   += 1000.0f * dt / camera.zoom;

        Clear(olc::VERY_DARK_GREY);
//        auto screenW = ScreenWidth();
//        auto screenH = ScreenHeight();
//        float scale = 0.2;
//        float speed = 100;
//        auto startingPos = -sprite->width()/2 * scale;

//        pos.x += speed * dt;
//        pos.y = (screenH/2 - sprite->height()*scale/2) * cos((pos.x-startingPos)*(2 * PI/(screenW-sprite->width()*scale))) + screenH/2;
//        if(pos.x > screenW + sprite->width()*scale/2)
//            pos.x = startingPos;
//        olc::vf2d centered = olc::vf2d(pos.x - sprite->width()*scale/2, pos.y - sprite->height()*scale/2);
//        sprite->pos = centered;
        sprite->translate3d(camera, {ScreenWidth(), ScreenHeight()});
//        DrawDecal(sprite->pos, sprite->decal, olc::vf2d(scale, scale));
        auto points = sprite->getWarpedPoints();
        DrawWarpedDecal(sprite->decal, points);
        DrawStringDecal({0,0}, "Zoom: " + std::to_string(camera.zoom));
        for(int i=0; i<4; i++)
            std::cout<<"Point"<<i<<"x:"<<points[i].x<<"    "<<"y:"<<points[i].y<<"\n";
        return true;
    }
};


int main()
{
    Example demo;
    if (demo.Construct(680, 340, 2, 2, false, true))
        demo.Start();

    return 0;
}
