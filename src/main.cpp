#include "Core/application.h"
#include "Core/scene.h"
#include "Core/gui.h"
#include "Renderer/renderpipeline.h"
#include "Core/instrumentation.h"
#include "Core/yamlserialization.h"
#include "Game/game.h"

int main(void)
{
    //TODO: add initialization tests everywhere and setup some defaults like camera etc
    App::init(1200, 720);
    App::setVsync(0);

    auto test3 = std::make_shared<Game>();
    RenderPipeline renderer;
    while (!App::shouldClose())
    {
        {
            PROFILE_SCOPE("CPU");
            AssetManager::checkForChanges();
            AssetManager::tryReloadAssets();
            imguiBegin();

            float dt = App::getTimeStep();
            test3->update(dt);
            test3->onUpdate(dt);
        }
        {
            PROFILE_SCOPE("GPU");
            renderer.drawScene(test3);
            imguiEnd();
            App::submitFrame();
        }
    }
    renderer.serialize();
}
