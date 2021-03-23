#include "asset_manager.h"
#include <chrono>

int main()
{
    AssetManager* assetManager = new AssetManager();
    auto spr = std::make_shared<Sprite>("../assets/img/test.png");
    assetManager->addAsset(spr);

    while(true){

        assetManager->checkForChanges();
        assetManager->tryReloadAssets();

        // Maybe we should add a timer that reloads an asset only after we are
        // sure that the other application is done with it

        // There is a timing issue right now some applications want to delete
        // and recreate a file eg. discarding changes in github,
        // if we try to reload file at a bad moment we will fail
    }

    return 0;
}
