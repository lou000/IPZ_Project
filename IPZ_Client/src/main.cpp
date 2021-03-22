#include "asset_manager.h"

int main()
{
    AssetManager* assetManager = new AssetManager();
    std::shared_ptr<Sprite> spr = std::make_shared<Sprite>(Sprite("../assets/img/test.png"));
    assetManager->addAsset(spr);

    while(true){
        assetManager->checkForChanges();
        assetManager->tryReloadAssets();
    }

    return 0;
}
