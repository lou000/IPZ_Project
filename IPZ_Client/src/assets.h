#pragma once
#include <filesystem>
#include <iostream>
#include "utilities.h"
#include "stb_image_resize.h"
#include "glm.hpp"


using namespace glm;

class Asset{
public:
    std::filesystem::path path;
    virtual void doReload() = 0;
    bool rld = false;

};

class Sprite : public Asset
{
public:
    Sprite() = default;
    Sprite(const std::filesystem::path& path);
    ~Sprite();
    uint16 width  = 0;
    uint16 height = 0;
    u8vec4* data  = nullptr;

public:
    bool loadFromFile(const std::filesystem::path& path);
    virtual void doReload() override;
};


