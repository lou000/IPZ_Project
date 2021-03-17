#include "windows.h"
#include "unordered_set"
#include "stb_image.h"
#include "stb_image_resize.h"
#include <filesystem>
#include <iostream>
#define UNUSED(x) (void)(x)

struct Dir{
    HANDLE handle;
    OVERLAPPED overlapped;
    FILE_NOTIFY_INFORMATION buffer[2][512];

    std::filesystem::path path;
    std::unordered_set<std::string> watchedFiles; //this should be in asset class

    int bufferIndex = 0;

    bool operator==(const Dir &other) const
    {
        return (path == other.path);
    }
};

struct AssetManager{
    // For now the hotloader functionality is run on the main thread, if it turns out to be
    // too inefficiant we can always move it to separate thread
    std::unordered_set<Dir*> dirs;
    std::unordered_set<std::string> reloadList;

    void addFile(const std::string& filePath);
    void removeFile(const std::string& filePath);
    void checkForChanges();
};

void AssetManager::addFile(const std::string& filePath)
{
    //TODO: error logging
    auto path = std::filesystem::path(filePath);
    Dir* dir =  new Dir();
    dir->path = path.remove_filename();
    dir->watchedFiles.insert(path.string());
    auto r = dirs.insert(dir);
    if(r.second) //If the dir was succesfully added to the set then setup the watch
    {
        const WCHAR *dirPath = dir->path.c_str();

        dir->handle = CreateFileW(
            dirPath,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ/*|FILE_SHARE_DELETE*/|FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS  | FILE_FLAG_OVERLAPPED ,
            NULL);

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
    for(auto dir : dirs)
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
            //this is the part where we need to see if file is an asset and add path+filename to reloadList
            std::wstring filename(pFileNotify->FileName, pFileNotify->FileNameLength / sizeof(WCHAR));
            std::wcout<<filename<<" changed\n";

            if (!pFileNotify->NextEntryOffset)
                break;
            pFileNotify = (FILE_NOTIFY_INFORMATION *)((PBYTE)pFileNotify + pFileNotify->NextEntryOffset);
        }
    }
}

