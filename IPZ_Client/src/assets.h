#pragma once
#include <filesystem>
#include "stb_image.h"
#include "stb_image_resize.h"
#include "glm.hpp"

class Asset{
    std::filesystem::path path;
    bool rld = false;
public:
    void reload(){rld = true;}
    virtual void doReload() = 0;
};

class Sprite : public Asset
{
    uint16_t width      = 0;
    uint16_t height     = 0;
    uint16_t channels   = 0;
    glm::uvec4* data = nullptr;

public:
    bool loadFromFile(char* file);
    virtual void doReload() override;
};

bool Sprite::loadFromFile(char* file){
    int w = 0, h = 0, ch = 0;
    if(!std::filesystem::exists(file))
        return false;
    data = (glm::uvec4*) stbi_load(file, &w, &h, &ch, 4);
    width = w;
    height = h;
    channels = ch;
    return true;
}
