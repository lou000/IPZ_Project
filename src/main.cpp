#include "Core/application.h"
#include "Core/scene.h"
#include "Core/gui.h"
#include "Renderer/renderpipeline.h"
#include "Tests/testconnect4.h"
#include"Core/yamlserialization.h"

int main(void)
{
    //TODO: add initialization tests everywhere and setup some defaults like camera etc
    App::init(1200, 800);
    App::setVsync(0);

    // FPS counter should go to App
    float dtSum = 0;
    int frameCount = 0;
    auto test3 = std::make_shared<TestConnect4>();
    Serializer::deserializeScene(test3, "../Config/testConnect4.pc");
    RenderPipeline renderer;
    while (!App::shouldClose())
    {
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();
        imguiBegin();

        float dt = App::getTimeStep();
        dtSum+=dt;
        frameCount++;
        if (frameCount == 20)
        {
            char title[50];
            double ms = dtSum/double(frameCount) * 1000;
            sprintf_s(title, "Test - FPS: %.1f (%.2fms)", 1/ms*1000, ms);
            App::setWindowTitle(title);
            frameCount = 0;
            dtSum = 0;
        }

        test3->activeCamera()->onUpdate(dt);
        test3->onUpdate(dt);
        renderer.drawScene(test3);
        imguiEnd();
        App::submitFrame();
    }
//    Serializer::serializeScene(test3, "../Config/scene.pc");
    renderer.serialize();
}
