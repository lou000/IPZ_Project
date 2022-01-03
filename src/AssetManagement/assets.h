#pragma once
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include "../Core/utilities.h"
#include "../Renderer/mesh.h"


using namespace glm;

enum AssetType{
    texture,
    shaderFile,
    meshFile,
};

class VertexArray;
class AssetManager;

// TODO: make utility functions toTexture() etc to quickly cast
class Asset{
    friend class AssetManager;
public:
    virtual bool doReload() = 0;

protected:
    AssetType assetType;
    bool reloadScheduled = false;
    std::filesystem::path path;
};



class Texture : public Asset
{
public:
    Texture(uint width, uint height, uint depth = 1, GLenum formatInternal = GL_RGBA8, uint samples = 1, bool loadDebug = false);
    Texture(const std::filesystem::path& path, uint samples = 1);
    ~Texture();

    virtual bool doReload() override;
    void setTextureData(void* data, size_t size);
    vec3 getDimensions();
    size_t getSize();
    GLenum formatInternal(){return m_formatInternal;}
    void resize(vec3 size);
    void bind(uint slot);
    void bindLayer(uint slot, uint layer);
    uint id() {return m_id;}
    uint selectedLayer(){return m_selectedLayer;}
    void selectLayerForNextDraw(uint layer);

private:
    uint m_width   = 0;
    uint m_height  = 0;
    uint m_depth   = 1;
    uint m_id      = 0;
    uint m_samples = 1;
    GLenum m_formatInternal = 0;

    // this is a workaround for batchrenderer to draw array textures
    uint m_selectedLayer = 0;

    void initTexture();
    void initTexture3D();
    void* loadFromFile(const std::filesystem::path& path);
    void clear(vec3 color);
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

    virtual bool doReload() override;
    const std::string& shaderName(){return m_shaderName;}

private:
    const std::string m_shaderName;
    ShaderType type;
    char* loadFile();
    bool getTypeFromFile();
    char* data;
};

class Model : public Asset
{

public:
    Model(const std::filesystem::path& path);

    virtual bool doReload() override;
    std::vector<std::shared_ptr<Mesh>> meshes(){return m_meshes;}
    AABB boundingBox(){return m_boundingBox;}

private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    AABB m_boundingBox;
    bool loadModel();

};



