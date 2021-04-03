#pragma once
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include "utilities.h"
#include "shader.h"
#include "glm.hpp"
#include "gl.h"


using namespace glm;

enum AssetType{
    texture,
    shaderFile
};


class Asset{
    friend class AssetManager;
public:
    virtual void doReload() = 0;

protected:
    AssetType assetType;
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



class ShaderFile : public Asset
{
    friend class Shader;
    enum ShaderType{
        vertex      = GL_VERTEX_SHADER,
        tessControl = GL_TESS_CONTROL_SHADER,
        tessEval    = GL_TESS_EVALUATION_SHADER,
        geometry    = GL_GEOMETRY_SHADER,
        fragment    = GL_FRAGMENT_SHADER,
        compute     = GL_COMPUTE_SHADER
    };

public:
    ShaderFile(const std::filesystem::path& path, const std::string shaderName);
    ShaderFile(const std::filesystem::path& path, ShaderType type, const std::string shaderName);

    const std::string& shaderName(){return m_shaderName;}
    void doReload() override;

private:
    const std::string m_shaderName;
    ShaderType type;
    char* loadFile();
    bool getTypeFromFile();
    char* data;
};



