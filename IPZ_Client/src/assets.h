#pragma once
#include <filesystem>
#include <iostream>
#include "utilities.h"
#include "glm.hpp"
#include "gl.h"


using namespace glm;

class Asset{
    friend struct AssetManager;
public:
    virtual void doReload() = 0;
protected:
    bool reloadScheduled = false;
    std::filesystem::path path;

};

class Texture : public Asset
{
public:
    Texture(GLenum format, GLenum formatInternal, uint32 width, uint32 height);
    Texture(const std::filesystem::path& path);
    ~Texture();

private:
    uint32 m_width  = 0;
    uint32 m_height = 0;
    uint32 m_id     = 0;
    GLenum m_format = 0;
    GLenum m_formatInternal = 0;

    void* data  = nullptr;

public:
    virtual void doReload() override;
    void setTextureData(void* data);

private:
    void initTexture();
    bool loadFromFile(const std::filesystem::path& path);
    void loadDebugTexture(GLenum format, GLenum formatInternal, uint32 width, uint32 height);
};


