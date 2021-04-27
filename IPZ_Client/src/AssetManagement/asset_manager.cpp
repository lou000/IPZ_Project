#include "asset_manager.h"
#include "ctime"

void AssetManager::x_addAsset(std::shared_ptr<Asset> asset)
{
    fileAssets.insert({asset->path, asset});
    auto dir =  std::make_shared<Dir>();
    auto p = asset->path;
    dir->path = p.remove_filename();
    auto r = dirs.insert({dir->path, dir});

    //If directory doesnt exist add it to map
    if(r.second)
    {
        dir->assets.insert({asset->path, asset});
        addDirWatch(dir);
    }
    else // if it does add asset to existing one
    {
        auto eDir = dirs[dir->path];
        eDir->assets.insert({asset->path, asset});
    }
}

std::shared_ptr<Asset> AssetManager::x_getAsset(const std::filesystem::path& path)
{
    if(fileAssets.find(path) == fileAssets.end())
    {
        WARN("AssetManager: There is no asset named %s.", path.string().c_str());
        return nullptr;
    }
    else
        return fileAssets.at(path);
}

void AssetManager::x_removeAsset(const std::filesystem::path &assetPath)
{
    //TODO: DAAAAAAAWIDDD dla Ciebie.
    // usunąć asset z listy "fileAssets" i z listy "assets" w dir
    // jezeli to byl jedyny asset w dir->assets to usunac dir
    // jezeli to byl shaderFile to odmowic,
    // usuwamy shaderFile usuwając shader nie osobno
}

void AssetManager::x_addShader(std::shared_ptr<Shader> shader)
{
    shaders.insert({shader->name, shader});
    for(auto& file : shader->files)
        x_addAsset(file);
}

void AssetManager::x_removeShader(int id)
{
    //TODO: DAAAAAAAWIDDD dla Ciebie.
    // tutaj trzeba usunac shader, i zrobic _removeAsset na wszystkich
    // shaderFile z nim powiązanych
}

std::shared_ptr<Shader> AssetManager::x_getShader(const std::string &name)
{
    if(shaders.find(name) == shaders.end())
        return nullptr;
    else
        return shaders.at(name);
}

void AssetManager::addDirWatch(std::shared_ptr<Dir> dir)
{

    auto pathStr = dir->path.native();
    const WCHAR* dirPath = pathStr.c_str();

    dir->handle = CreateFileW(
        dirPath,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ/*|FILE_SHARE_DELETE|*/|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED ,
        NULL);

    //the buffer may overflow we should handle this, windows is providing us with info about this
    ZeroMemory(dir->buffer, sizeof(dir->buffer));
    ZeroMemory(&dir->overlapped, sizeof(dir->overlapped));
    dir->overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    auto ret = ReadDirectoryChangesW(
             dir->handle,
             &dir->buffer[dir->bufferIndex],
             sizeof(dir->buffer[dir->bufferIndex]),
             FALSE,
             FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
             NULL,
             &dir->overlapped,
             NULL);
    if(!ret)
        WARN("ReadDirectoryChangesW failed with %ld", GetLastError());

}

void AssetManager::x_checkForChanges()
{
    // TODO: error logging
    for(auto& [path, dir] : dirs)
    {
        if (!dir->overlapped.hEvent)
            continue;

        DWORD dwObj = WaitForSingleObject(dir->overlapped.hEvent, 0);
        if (dwObj != WAIT_OBJECT_0)
            continue;

        DWORD dwNumberbytes;
        GetOverlappedResult(dir->handle, &dir->overlapped, &dwNumberbytes, FALSE);

        FILE_NOTIFY_INFORMATION *pFileNotify = dir->buffer[dir->bufferIndex];
        dir->bufferIndex = !dir->bufferIndex;

        ReadDirectoryChangesW(
            dir->handle,
            &dir->buffer[dir->bufferIndex],
            sizeof(dir->buffer[dir->bufferIndex]),
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
            NULL,
            &dir->overlapped,
            NULL);

        while(true) {
            std::wstring filename(pFileNotify->FileName, pFileNotify->FileNameLength / sizeof(WCHAR));
            auto fullPath = dir->path;
            fullPath.replace_filename(filename);

            if(dir->assets.count(fullPath) != 0)
            {
                auto asset = dir->assets[fullPath];
                if(asset->reloadScheduled != true)
                {
                    if(timeFirstChange == 0)
                        timeFirstChange = std::clock();
                    asset->reloadScheduled = true;
                    LOG("Asset '%s' found and marked for reload", fullPath.string().c_str());
                }
            }

            if (!pFileNotify->NextEntryOffset)
                break;
            pFileNotify = (FILE_NOTIFY_INFORMATION *)((PBYTE)pFileNotify + pFileNotify->NextEntryOffset);
        }
    }
}

void AssetManager::x_tryReloadAssets()
{
    // Let the app thats making changes some time to process the file
    double msPassed = (std::clock() - timeFirstChange)/(double)(CLOCKS_PER_SEC / 1000);
    if(timeFirstChange==0 || msPassed<500)
        return;
    else
        timeFirstChange = 0;

    // Just in case someone changed multiple shader files,
    // this set below is for uniqueness.
    std::unordered_set<std::shared_ptr<Shader>> shadersToReload;
    for(auto& asset : fileAssets)
    {
        if(asset.second->reloadScheduled)
        {
            if(asset.second->doReload())
            {
                LOG("Asset %s reloaded.", asset.second->path.string().c_str());
                asset.second->reloadScheduled = false;
            }
            if(asset.second->assetType == AssetType::shaderFile)
            {
                auto sF = std::dynamic_pointer_cast<ShaderFile>(asset.second);
                // You can add a shaderFile to assets without compiling it to a shader. But why would you?
                if(shaders.find(sF->shaderName()) == shaders.end())
                    WARN("AssetManager: Reloaded shader file that is not bound to a shader. This will have no effect.");
                else
                    shadersToReload.insert(shaders[sF->shaderName()]);
            }
        }
    }
    for(auto shader : shadersToReload)
        shader->compile();
}
