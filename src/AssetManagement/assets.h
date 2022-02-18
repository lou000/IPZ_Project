#pragma once
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>
#include "../Core/utilities.h"
#include "../Renderer/mesh.h"


using namespace glm;

enum AssetType{
    texture,
    shaderFile,
    meshFile,
    audioBuffer
};

class VertexArray;
class AssetManager;

class Asset{
    friend class AssetManager;
public:
    virtual bool doReload() = 0;
    std::string getName();
    bool hasFile(){return !m_path.empty();}

protected:
    Asset(AssetType type) : assetType(type){}
    AssetType assetType;
    bool reloadScheduled = false;
    std::filesystem::path m_path;
    std::string m_name;
};

class AudioBuffer : public Asset
{
public:
    AudioBuffer(const std::filesystem::path& path);
    ~AudioBuffer();
    virtual bool doReload() override;
    ALuint bufferID(){return m_bufferID;}
    void setData(void *data, size_t size, ALenum format, uint samplerate);

private:
    void init();
    void* loadFromFile();
    ALuint m_bufferID   = 0;
    ALenum m_ALFormat   = 0;
    uint m_channels     = 0;
    uint m_samplerate   = 0;
    uint64 m_framecount = 0;
};

class Texture : public Asset
{
public:
    Texture(uint width, uint height, uint depth = 1, GLenum formatInternal = GL_RGBA8,
            GLenum textureWrap = GL_CLAMP_TO_EDGE, uint samples = 1, bool loadDebug = false);
    Texture(const std::filesystem::path& path, uint samples = 1);
    ~Texture();

    virtual bool doReload() override;
    void setData(void* data, size_t size);
    vec3 getDimensions();
    size_t getSize();
    GLenum glFormatSized(){return m_glFormatSized;}
    GLenum glType(){return m_glType;}
    void resize(vec3 size);
    void bind(uint slot);
    void getImage(uint64 size, void* memory);
    void bindLayer(uint slot, uint layer);
    uint id() {return m_id;}
    uint selectedLayer(){return m_selectedLayer;}
    void selectLayerForNextDraw(uint layer);
    void copyTo(std::shared_ptr<Texture> target, vec3 srcXYZ, vec3 destXYZ, vec3 size);
    void clear(vec3 color);

private:
    uint m_width   = 0;
    uint m_height  = 0;
    uint m_depth   = 1;
    uint m_id      = 0;
    uint m_samples = 1;
    GLenum m_glType = 0;
    GLenum m_glFormatSized = 0;
    GLenum m_textureWrap = 0;

    // this is a workaround for batchrenderer to draw array textures
    uint m_selectedLayer = 0;

    void initTexture();
    void initTexture3D();
    void* loadFromFile(const std::filesystem::path& path);
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
    ShaderFile(const std::filesystem::path& path, const std::string programName);
    ShaderFile(const std::filesystem::path& path, ShaderType type, const std::string programName);

    virtual bool doReload() override;
    const std::string& shaderName(){return m_programName;}

private:
    const std::string m_programName;
    ShaderType type;
    std::string loadFile();
    bool getTypeFromFileName();
    std::string text;
};

class Model : public Asset
{

public:
    Model(const std::string& name, bool loadDebug); //TODO: debug model
    Model(const std::filesystem::path& path);
    Model(std::string name, std::vector<std::shared_ptr<Mesh>> meshes);

    virtual bool doReload() override;
    std::vector<std::shared_ptr<Mesh>> meshes(){return m_meshes;}
    AABB boundingBox(){return m_boundingBox;}

    static std::shared_ptr<Model> makeUnitPlane();

private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    AABB m_boundingBox;
    bool loadModel();
};



