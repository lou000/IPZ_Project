#pragma once
#include "graphics.h"

using namespace IPZ;
class Game : public olc::PixelGameEngine
{
public:
    Game()
    {
        sAppName = "Unnamed Game Client";
        camera.screenSize = vec2(ScreenWidth(), ScreenHeight());
    }

public:
    Sprite* sprite;
    olc::Sprite* a;
    olc::GFX3D::mesh meshQuad;
    olc::GFX3D::mesh meshCube;
    olc::GFX3D::PipeLine pipe;
    Sprite* test;
    Camera camera;
    float cubeRotY = 0;

public:
    bool OnUserCreate() override
    {
        sprite = new Sprite("../assets/img/test.png");
        a = new olc::Sprite(sprite->sprite->width, sprite->sprite->height);
        SetDrawTarget(a);
        DrawSprite(0,0, sprite->sprite, 1, olc::Sprite::Flip::VERT);
        test = new Sprite("../assets/img/bomb.png");
        SetDrawTarget(nullptr);

        olc::GFX3D::ConfigureDisplay();
        meshQuad.tris =
        {
                { 0.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 1.0f, 0.0f, 1.0f,		 1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
                { 0.0f, 0.0f, 0.0f, 1.0f,  		1.0f, 1.0f, 0.0f, 1.0f,		 1.0f, 0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
        };
        meshCube.tris =
        {
            // SOUTH
            { 0.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 1.0f, 0.0f, 1.0f,		 1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
            { 0.0f, 0.0f, 0.0f, 1.0f,  		1.0f, 1.0f, 0.0f, 1.0f,		 1.0f, 0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },

            // EAST
            { 1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
            { 1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },

            // NORTH
            { 1.0f, 0.0f, 1.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
            { 1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },

            // WEST
            { 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
            { 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },

            // TOP
            { 0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
            { 0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },

            // BOTTOM
            { 1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },
            { 1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, olc::WHITE, olc::WHITE, olc::WHITE },

        };

        return true;
    }

    bool OnUserUpdate(float dt) override
    {

        olc::GFX3D::mat4x4 rotX = olc::GFX3D::Math::Mat_MakeRotationX(1.0f * dt);
        olc::GFX3D::mat4x4 rotXn = olc::GFX3D::Math::Mat_MakeRotationX(-1.0f * dt);
        // WS keys to tilt camera
        if (GetKey(olc::Key::W).bHeld) camera.dir = olc::GFX3D::Math::Mat_MultiplyVector(rotX, camera.dir);
        if (GetKey(olc::Key::S).bHeld) camera.dir = olc::GFX3D::Math::Mat_MultiplyVector(rotXn, camera.dir);

        olc::GFX3D::mat4x4 rotY = olc::GFX3D::Math::Mat_MakeRotationY(1.0f * dt);
        olc::GFX3D::mat4x4 rotYn = olc::GFX3D::Math::Mat_MakeRotationY(-1.0f * dt);

        // DA Keys to manually rotate camera
        if (GetKey(olc::Key::D).bHeld) camera.dir = olc::GFX3D::Math::Mat_MultiplyVector(rotY, camera.dir);
        if (GetKey(olc::Key::A).bHeld) camera.dir = olc::GFX3D::Math::Mat_MultiplyVector(rotYn, camera.dir);

        // QZ Keys to zoom in or out
        if (GetKey(olc::Key::Q).bHeld) camera.pos.z += 2.0f * dt;
        if (GetKey(olc::Key::Z).bHeld) camera.pos.z -= 2.0f * dt;

        //pos
        if (GetKey(olc::Key::LEFT).bHeld) camera.pos.x   -= 2.0f * dt;
        if (GetKey(olc::Key::RIGHT).bHeld) camera.pos.x  += 2.0f * dt;
        if (GetKey(olc::Key::DOWN).bHeld) camera.pos.z   -= 2.0f * dt;
        if (GetKey(olc::Key::UP).bHeld) camera.pos.z     += 2.0f * dt;


        // DA Keys to manually rotate camera
        if (GetKey(olc::Key::K).bHeld) cubeRotY += 1.0f * dt;
        if (GetKey(olc::Key::L).bHeld) cubeRotY -= 1.0f * dt;

        Clear(olc::VERY_DARK_GREY);
        olc::GFX3D::ClearDepth();

        olc::GFX3D::vec3d vLookTarget = olc::GFX3D::Math::Vec_Add(camera.pos, camera.dir);

        pipe.SetCamera(camera.pos, vLookTarget, vec3::up());
        pipe.SetProjection(camera.fov, (float)ScreenHeight()/(float)ScreenWidth(), camera.clipNear,
                           camera.clipFar, 0, 0, (float)ScreenWidth(), (float)ScreenHeight());
        olc::GFX3D::mat4x4 matWorld = olc::GFX3D::Math::Mat_MakeRotationY(cubeRotY);
        pipe.SetTransform(matWorld);
        pipe.SetTexture(a);
        olc::GFX3D::vec3d lightdir = { 1.0f, 1.0f, -1.0f };
        pipe.SetLightSource(0, olc::GFX3D::LIGHT_AMBIENT, olc::Pixel(100,100,100), { 0,0,0 }, lightdir);
        pipe.SetLightSource(1, olc::GFX3D::LIGHT_DIRECTIONAL, olc::WHITE, { 0,0,0 }, lightdir);
        pipe.Render(meshCube.tris, olc::GFX3D::RENDER_CULL_CW | olc::GFX3D::RENDER_DEPTH | olc::GFX3D::RENDER_TEXTURED | olc::GFX3D::RENDER_LIGHTS);

//        drawTestGrid(camera);
//        vec2 mouse = GetMousePos();
//        test->pos = mouse;
//        test->translateOrthographic(camera);
//        sprite->translateOrthographic(camera);
//        DrawWarpedDecal(sprite->decal, sprite->xyOrtho());
//        DrawStringDecal({0,0}, "Zoom: " + std::to_string(camera.zoom));
//        DrawWarpedDecal(test->decal, test->xyOrtho());
//        sprite->debugPrintOrtho();
//        std::cout<<test->pos<<"\n";
        return true;
    }
//    void drawTestGrid(Camera camera)
//    {
//        for(int i = -100; i<=100; i+=10)
//        {
////            float x = i * camera.zoom - camera.pos.x;
////            float y = i * camera.zoom - camera.pos.y;
////            float maxX = 1000 * camera.zoom - camera.pos.x;
////            float maxY = 1000 * camera.zoom - camera.pos.y;


//            vec2 vfirst =  {(float)i,100.0f};
//            vec2 vsecond = {(float)i,-100.0f};

//            vec2 hfirst =  {100.0f, (float)i};
//            vec2 hsecond = {-100.0f, (float)i};


//            vfirst = translatePointToScreen(camera, vfirst);
//            vsecond = translatePointToScreen(camera, vsecond);

//            hfirst = translatePointToScreen(camera, hfirst);
//            hsecond = translatePointToScreen(camera, hsecond);

//            DrawLine(vfirst.x, vfirst.y, vsecond.x, vsecond.y);
//            DrawLine(hfirst.x, hfirst.y, hsecond.x, hsecond.y);
//        }
//    }
};
