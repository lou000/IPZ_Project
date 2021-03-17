#include "windows.h"
#include "unordered_set"
#include "stb_image.h"
#include "stb_image_resize.h"
#include <filesystem>
#define UNUSED(x) (void)(x)


enum AssetType{
    TEXT_FILE,
    TEXTURE,
};

struct Dir{
    std::filesystem::path path;
    HANDLE handle;
    FILE_NOTIFY_INFORMATION buffer[2][512];
    int bufferIndex = 0;
};

struct AssetManager{
    std::unordered_set<std::filesystem::path> files;
    std::unordered_set<Dir> dirs;
    std::unordered_set<std::filesystem::path> loadList;

    void addFile(const std::string& fileName, AssetType type);
};

void AssetManager::addFile(const std::string& fileName, AssetType type)
{
    UNUSED(type);
    auto path = std::filesystem::path(fileName);
    Dir dir;
    dir.path = path.remove_filename();
    files.insert(path);
    auto r = dirs.insert(dir);
    if(r.second) //If the dir was succesfully added to the set then setup the watch
    {
        const WCHAR *dirPath = dir.path.c_str();

        dir.handle = CreateFileW(
            dirPath, // dir
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ/*|FILE_SHARE_DELETE*/|FILE_SHARE_WRITE,  // shouldnt allow deletions?
            NULL, // security descriptor
            OPEN_EXISTING, // how to create
            FILE_FLAG_BACKUP_SEMANTICS  | FILE_FLAG_OVERLAPPED , // file attributes
            NULL); // file with attributes to copy
        ZeroMemory(dir.buffer, sizeof(dir.buffer));

        ReadDirectoryChangesW(
                 dir.handle, /* handle to directory */
                 &dir.buffer[dir.bufferIndex], /* read results buffer */
                 sizeof(dir.buffer[dir.bufferIndex]), /* length of buffer */
                 FALSE, /* monitoring option */
                 FILE_NOTIFY_CHANGE_LAST_WRITE, /* filter conditions */
                 NULL, /* bytes returned */
                 NULL, /* overlapped buffer */
                 NULL); /* completion routine */
    }
}

