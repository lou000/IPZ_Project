#pragma once
#include "windows.h"
#include "assets.h"
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <memory>
#include <map>
#define UNUSED(x) (void)(x)

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
    std::unordered_set<std::shared_ptr<Dir>> dirs;
    void addFileWatch(const std::filesystem::path& filePath);
    void removeFileWatch(const std::filesystem::path& filePath);

};

void AssetManager::addAsset(std::shared_ptr<Asset> asset)
{
    assets.insert({asset->path, asset});
    addFileWatch(asset->path);
}

void AssetManager::addFileWatch(const std::filesystem::path& filePath)
{
    //TODO: error logging
    auto dir =  std::make_shared<Dir>();
    dir->path = filePath;
    dir->path = dir->path.remove_filename();

    auto r = dirs.insert(dir);

    if(r.second) //If the dir was succesfully added to the set then setup the watch
    {
        const WCHAR* dirPath = dir->path.wstring().c_str();

        dir->handle = CreateFileW(
            dirPath,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ/*|FILE_SHARE_DELETE*/|FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS  | FILE_FLAG_OVERLAPPED ,
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
                 FILE_NOTIFY_CHANGE_LAST_WRITE,
                 NULL,
                 &dir->overlapped,
                 NULL);
        if(!ret)
            wprintf(L"ReadDirectoryChangesW failed with 0x%x\n", GetLastError());
    }
}

void AssetManager::checkForChanges()
{
    // TODO: error logging
    for(auto& dir : dirs)
    {
        if (!dir->overlapped.hEvent)
            break;

        DWORD dwObj = WaitForSingleObject(dir->overlapped.hEvent, 0);
        if (dwObj != WAIT_OBJECT_0)
            break;

        DWORD dwNumberbytes;
        GetOverlappedResult(dir->handle, &dir->overlapped, &dwNumberbytes, FALSE);

        FILE_NOTIFY_INFORMATION *pFileNotify = dir->buffer[dir->bufferIndex];
        dir->bufferIndex = !dir->bufferIndex;

        ReadDirectoryChangesW(
            dir->handle,
            &dir->buffer[dir->bufferIndex],
            sizeof(dir->buffer[dir->bufferIndex]),
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL,
            &dir->overlapped,
            NULL);

        while(true) {
            std::wstring filename(pFileNotify->FileName, pFileNotify->FileNameLength / sizeof(WCHAR));
            auto fullPath = dir->path.replace_filename(filename);
            if(dir->assets.count(fullPath)>0)
            {
                dir->assets[fullPath]->reload();
                std::cout<<"Asset '"<<fullPath<<"' found and marked for reload";
            }
            else
            {
                std::cout<<"There is no asset '"<<fullPath<<"'";
            }
            std::cout<<fullPath<<" changed\n";

            if (!pFileNotify->NextEntryOffset)
                break;
            pFileNotify = (FILE_NOTIFY_INFORMATION *)((PBYTE)pFileNotify + pFileNotify->NextEntryOffset);
        }
    }
}

void AssetManager::tryReloadAssets()
{
    for(auto asset : assets)
        asset.second->reload();
}

