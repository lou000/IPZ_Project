#pragma once
#include <windows.h>
#include <assimp/Importer.hpp>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <memory>
#include <map>
#include "assets.h"
#include "../Core/utilities.h"


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

    static void addAsset(std::shared_ptr<Asset> asset){getInstance().x_addAsset(asset);}
    static void removeAsset(const std::filesystem::path& assetPath){getInstance().x_removeAsset(assetPath);}
    static std::shared_ptr<Asset> getAsset(const std::filesystem::path& name) {return getInstance().x_getAsset(name);}

    static void addShader(std::shared_ptr<Shader> shader){getInstance().x_addShader(shader);}
    static void removeShader(int id){getInstance().x_removeShader(id);}
    static std::shared_ptr<Shader> getShader(const std::string& name) {return getInstance().x_getShader(name);}

    static Assimp::Importer* getAssimpImporter(){return getInstance().x_getAssimpImporter();}
    static void tryReloadAssets() {getInstance().x_tryReloadAssets();}
    static void checkForChanges() {getInstance().x_checkForChanges();}

private:
    std::clock_t timeFirstChange = 0;
    Assimp::Importer importer;
    Assimp::Importer* x_getAssimpImporter(){return &importer;}

    void x_addAsset(std::shared_ptr<Asset> asset);
    void x_removeAsset(const std::filesystem::path& assetPath);
    std::shared_ptr<Asset> x_getAsset(const std::filesystem::path& path);

    void x_addShader(std::shared_ptr<Shader> shader);
    void x_removeShader(uint id);
    std::shared_ptr<Shader> x_getShader(const std::string& name);

    void x_tryReloadAssets(); // files may still be locked by application making changes
    void x_checkForChanges(); // run this as often as convieniant

    std::map<std::string, std::shared_ptr<Shader>> shaders;
    std::map<std::filesystem::path, std::shared_ptr<Asset>> fileAssets;
    std::map<std::filesystem::path, std::shared_ptr<Dir>> dirs;
    void addDirWatch(std::shared_ptr<Dir> dir);
    void removeDirWatch(std::shared_ptr<Dir> dir);

};


