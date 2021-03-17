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
    std::unordered_set<std::string> watchedFiles;

    int bufferIndex = 0;

    bool operator==(const Dir &other) const
    {
        return (path == other.path);
    }
};

struct AssetManager{
    std::unordered_set<Dir*> dirs;
    std::unordered_set<std::string> reloadList;

    void addFile(const std::string& filePath);
    void checkForChanges();
};

void AssetManager::addFile(const std::string& filePath)
{
    auto path = std::filesystem::path(filePath);
    Dir* dir =  new Dir();
    dir->path = path.remove_filename();
    dir->watchedFiles.insert(path.string());
    auto r = dirs.insert(dir);
    if(r.second) //If the dir was succesfully added to the set then setup the watch
    {
        const WCHAR *dirPath = dir->path.c_str();

        dir->handle = CreateFileW(
            dirPath, // dir
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ/*|FILE_SHARE_DELETE*/|FILE_SHARE_WRITE,  // shouldnt allow deletions?
            NULL, // security descriptor
            OPEN_EXISTING, // how to create
            FILE_FLAG_BACKUP_SEMANTICS  | FILE_FLAG_OVERLAPPED , // file attributes
            NULL); // file with attributes to copy
        ZeroMemory(dir->buffer, sizeof(dir->buffer));
        ZeroMemory(&dir->overlapped, sizeof(dir->overlapped));
        dir->overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        auto ret = ReadDirectoryChangesW(
                 dir->handle, // handle to directory
                 &dir->buffer[dir->bufferIndex], // read results buffer
                 sizeof(dir->buffer[dir->bufferIndex]), // length of buffer
                 FALSE, /* monitoring option */
                 FILE_NOTIFY_CHANGE_LAST_WRITE, /* filter conditions */
                 NULL, /* bytes returned */
                 &dir->overlapped, /* overlapped buffer */
                 NULL); /* completion routine */
        if(!ret)
            wprintf(L"ReadDirectoryChangesW failed with 0x%x\n", GetLastError());
    }
}

void AssetManager::checkForChanges()
{
    for(auto dir : dirs)
    {
        if (!dir->overlapped.hEvent)
            break;

        DWORD dwObj = WaitForSingleObject(dir->overlapped.hEvent, 0);
        if (dwObj != WAIT_OBJECT_0)
            break;

        // Read the asynchronous result of the previous call to ReadDirectory
        DWORD dwNumberbytes;
        GetOverlappedResult(dir->handle, &dir->overlapped, &dwNumberbytes, FALSE);

        // Browse the list of FILE_NOTIFY_INFORMATION entries
        FILE_NOTIFY_INFORMATION *pFileNotify = dir->buffer[dir->bufferIndex];
        dir->bufferIndex = !dir->bufferIndex;

        // start a new asynchronous call to ReadDirectory in the alternate buffer
        ReadDirectoryChangesW(
            dir->handle, /* handle to directory */
            &dir->buffer[dir->bufferIndex], /* read results buffer */
            sizeof(dir->buffer[dir->bufferIndex]), /* length of buffer */
            FALSE, /* monitoring option */
            //FILE_NOTIFY_CHANGE_CREATION|
            FILE_NOTIFY_CHANGE_LAST_WRITE, /* filter conditions */
            NULL, /* bytes returned */
            &dir->overlapped, /* overlapped buffer */
            NULL); /* completion routine */

        while(true) {
            std::wstring filename(pFileNotify->FileName, pFileNotify->FileNameLength / sizeof(WCHAR));
            std::wcout<<filename<<" changed\n";


            // step to the next entry if there is one
            if (!pFileNotify->NextEntryOffset)
                break;
            pFileNotify = (FILE_NOTIFY_INFORMATION *)((PBYTE)pFileNotify + pFileNotify->NextEntryOffset);
        }
    }
}

