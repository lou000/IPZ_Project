#include "application.h"
#include "test1.h"
#include "testslidingpuzzle.h"
#include "testconnect4.h"

int main(void)
{
    //TODO: add initialization tests everywhere and setup some defaults like camera etc
    App::init(800, 800);
    App::setVsync(1);
    Renderer::init();

    // FPS counter should go to App
    float dtSum = 0;
    int frameCount = 0;

//    Test1 test = Test1();
//    TestSlidingPuzzle test = TestSlidingPuzzle();
    TestConnect4 test = TestConnect4();
    while (!App::shouldClose())
    {
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
        Renderer::getCamera()->onUpdate(dt);
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();


        test.onUpdate(dt);


        App::submitFrame();
    }
}
