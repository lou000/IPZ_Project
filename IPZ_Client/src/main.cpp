#include "asset_manager.h"
#include <chrono>

int main()
{
    using namespace std::literals;
    AssetManager* assetManager = new AssetManager();
    std::shared_ptr<Sprite> spr = std::make_shared<Sprite>("../assets/img/test.png");
    assetManager->addAsset(spr);
    std::chrono::steady_clock clock;
    auto prevTime = clock.now();

    while(true){

        auto now = clock.now();
        std::chrono::duration<double> diff = now-prevTime;
        assetManager->checkForChanges(); //this should happen often
        if(diff > 2000ms)
        {
            assetManager->tryReloadAssets(); //this shouldnt
            prevTime = now;
        }
    }

    return 0;
}
