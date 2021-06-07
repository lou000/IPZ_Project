#include "Core/application.h"
#include "Core/scene.h"
#include "Tests/test1.h"
#include "Tests/testslidingpuzzle.h"
#include "Tests/testconnect4.h"
#include "Tests/testrosenblat.h"
#include "Tests/testmlp.h"
#include "Tests/testga.h"
#include "Tests/testpendulum.h"

int main(void)
{
    //TODO: add initialization tests everywhere and setup some defaults like camera etc
    App::init(1200, 800);
    App::setVsync(0);

    // FPS counter should go to App
    float dtSum = 0;
    int frameCount = 0;

    std::vector<std::filesystem::path> shaderSrcs2 = {
        "../assets/shaders/default_mesh.fs",
        "../assets/shaders/default_mesh.vs"
    };
    AssetManager::addShader(std::make_shared<Shader>("mesh", shaderSrcs2));

    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/default_batch.fs",
        "../assets/shaders/default_batch.vs"
    };
    AssetManager::addShader(std::make_shared<Shader>("batch", shaderSrcs));

    Scene* test1 = new Test1();
    Scene* test2 = new TestSlidingPuzzle();
    Scene* test3 = new TestConnect4();
    Scene* test4 = new TestRosenblat();
    Scene* test5 = new TestMLP();
    Scene* test6 = new TestGA();
    Scene* test7 = new TestPendulum();

    Scene* currentTest = test7;
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
    FrameBuffer fbo = FrameBuffer(winSize.x, winSize.y, std::vector<FrameBufferAttachment>{colorAtt}, depthAtt, 16);
    while (!App::shouldClose())
    {
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

        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();

        if(App::getKeyOnce(GLFW_KEY_F1))
        {
            currentTest = test1;
            currentTest->onStart();
        }
        if(App::getKeyOnce(GLFW_KEY_F2))
        {
            currentTest = test2;
            currentTest->onStart();
        }
        if(App::getKeyOnce(GLFW_KEY_F3))
        {
            currentTest = test3;
            currentTest->onStart();
        }
        if(App::getKeyOnce(GLFW_KEY_F4))
        {
            currentTest = test4;
            currentTest->onStart();
        }
        if(App::getKeyOnce(GLFW_KEY_F5))
        {
            currentTest = test5;
            currentTest->onStart();
        }
        if(App::getKeyOnce(GLFW_KEY_F6))
        {
            currentTest = test6;
            currentTest->onStart();
        }
        if(App::getKeyOnce(GLFW_KEY_F7))
        {
            currentTest = test7;
            currentTest->onStart();
        }

        GraphicsContext::getCamera()->onUpdate(dt);
        currentTest->onUpdate(dt);
        fbo.blitToFrontBuffer();
        App::submitFrame();
    }
}
