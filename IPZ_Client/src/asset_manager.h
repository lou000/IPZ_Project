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

struct AssetManager{ // this right here should be a singleton
    // For now the hotloader functionality is run on the main thread, if it turns out to be
    // too inefficiant we can always move it to separate thread
    std::map<std::filesystem::path, std::shared_ptr<Asset>> assets;

public:
    void addAsset(std::shared_ptr<Asset> asset);
    void checkForChanges(); // run this as often as convieniant
    void tryReloadAssets(); // files may still be locked by application making changes

private:
    void addDirWatch(std::shared_ptr<Dir> dir);
    std::map<std::filesystem::path, std::shared_ptr<Dir>> dirs;
    void removeDirWatch(std::shared_ptr<Dir> dir);

};


