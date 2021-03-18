#pragma once
#include <filesystem>
#include "stb_image.h"
#include "stb_image_resize.h"

class Asset{
    std::filesystem::path path;
    bool rld = false;
public:
    void reload(){rld = true;}
    virtual void doReload() = 0;
};

class Sprite : public Asset
{
    uint16_t width;
    uint16_t height;


public:
    bool loadFromFile();
    virtual void doReload() override;
};
