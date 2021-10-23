#include "Core/application.h"
#include "Core/scene.h"
#include "Tests/test1.h"
#include "Tests/testslidingpuzzle.h"
#include "Tests/testconnect4.h"
#include "Tests/testrosenblat.h"
#include "Tests/testmlp.h"
#include "Tests/testga.h"
#include "Tests/testpendulum.h"
#include "Tests/testcontagionsim.h"

int main(void)
{
    //TODO: add initialization tests everywhere and setup some defaults like camera etc
    App::init(1200, 800);
    App::setVsync(0);

    // FPS counter should go to App
    float dtSum = 0;
    int frameCount = 0;


    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/default_batch.fs",
        "../assets/shaders/default_batch.vs"
    };
    AssetManager::addShader(std::make_shared<Shader>("batch", shaderSrcs));

    std::vector<std::filesystem::path> shaderSrcs2 = {
        "../assets/shaders/default_mesh.fs",
        "../assets/shaders/default_mesh.vs"
    };
    AssetManager::addShader(std::make_shared<Shader>("mesh", shaderSrcs2));

    auto test1 = Test1();
    auto test2 = TestSlidingPuzzle();
    auto test3 = TestConnect4();
    auto test4 = TestRosenblat();
    auto test5 = TestMLP();
    auto test6 = TestGA();
    auto test7 = TestPendulum();
    auto test8 = TestContagionSim();

    std::vector<Scene*> tests;
    tests.push_back(&test1);
    tests.push_back(&test2);
    tests.push_back(&test3);
    tests.push_back(&test4);
    tests.push_back(&test5);
    tests.push_back(&test6);
    tests.push_back(&test7);
    tests.push_back(&test8);


    Scene* currentTest = &test1;
    currentTest->onStart();


    FrameBufferAttachment colorAtt;
    colorAtt.type = GL_COLOR_ATTACHMENT0;
    colorAtt.format = GL_RGBA8;
    colorAtt.renderBuffer = true;

    FrameBufferAttachment depthAtt;
    depthAtt.type = GL_DEPTH_STENCIL_ATTACHMENT;
    depthAtt.format = GL_DEPTH24_STENCIL8;
    depthAtt.renderBuffer = true;

    auto winSize = App::getWindowSize();
    FrameBuffer fbo = FrameBuffer(winSize.x, winSize.y, {colorAtt}, depthAtt, 16);
    while (!App::shouldClose())
    {
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();

        fbo.bind();
        App::clearAll();
        float dt = App::getTimeStep();
        dtSum+=dt;
        frameCount++;
        if (frameCount == 20){
            char title[50];
            double ms = dtSum/double(frameCount) * 1000;
            sprintf_s(title, "Test - FPS: %.1f (%.2fms)", 1/ms*1000, ms);
            App::setWindowTitle(title);
            frameCount = 0;
            dtSum = 0;
        }

        for(uint i=0; i<tests.size(); i++)
        {
            if(App::getKeyOnce(GLFW_KEY_F1+i))
            {
                currentTest = tests.at(i);
                currentTest->onStart();
            }
        }

        GraphicsContext::getCamera()->onUpdate(dt);
        currentTest->onUpdate(dt);
        fbo.blitToFrontBuffer();
        App::submitFrame();
    }
}
