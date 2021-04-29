#include "Core/application.h"
#include "Tests/test1.h"
#include "Tests/testslidingpuzzle.h"
#include "Tests/testconnect4.h"

int main(void)
{
    //TODO: add initialization tests everywhere and setup some defaults like camera etc
    App::init(800, 800);
    App::setVsync(1);
    BatchRenderer::init();

    // FPS counter should go to App
    float dtSum = 0;
    int frameCount = 0;

//    Test1 test = Test1();
//    TestSlidingPuzzle test = TestSlidingPuzzle();
    TestConnect4 test = TestConnect4();

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
        App::clearAll();    //clear backbuffer
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
        BatchRenderer::getCamera()->onUpdate(dt);
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();


        test.onUpdate(dt);

        fbo.blitToFrontBuffer();
        App::submitFrame();
    }
}
