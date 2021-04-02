#pragma once
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <map>
#include "assets.h"
#include "utilities.h"


struct Dir{
    HANDLE handle;
    OVERLAPPED overlapped;
    FILE_NOTIFY_INFORMATION buffer[2][512];

    std::filesystem::path path;
    std::map<std::filesystem::path, std::shared_ptr<Asset>> assets;

    int bufferIndex = 0;

    bool operator==(const Dir &other) const
    {
        return (path == other.path);
    }
};

class AssetManager{
    // For now the hotloader functionality is run on the main thread, if it turns out to be
    // too inefficiant we can always move it to separate thread
    AssetManager() = default;
    static AssetManager& getInstance(){
        static AssetManager instance;
        return instance;
    }

public:
    AssetManager(AssetManager const&)    = delete;
    void operator=(AssetManager const&)  = delete;

    static void addAsset(std::shared_ptr<Asset> asset){getInstance()._addAsset(asset);}
    static void tryReloadAssets() {getInstance()._tryReloadAssets();}
    static void checkForChanges() {getInstance()._checkForChanges();}

private:
    void _addAsset(std::shared_ptr<Asset> asset);
    void _tryReloadAssets(); // files may still be locked by application making changes
    void _checkForChanges(); // run this as often as convieniant


    std::map<std::filesystem::path, std::shared_ptr<Asset>> assets;
    std::map<std::filesystem::path, std::shared_ptr<Dir>> dirs;
    void addDirWatch(std::shared_ptr<Dir> dir);
    void removeDirWatch(std::shared_ptr<Dir> dir);

};


