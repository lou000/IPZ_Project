#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdlib> //calloc

Texture::Texture(GLenum format, GLenum formatInternal, uint32 width, uint32 height)
    : m_width(width), m_height(height), m_format(format), m_formatInternal(formatInternal)
{
    assetType = AssetType::texture;
    initTexture();
    loadDebugTexture(format, formatInternal, width, height);
    setTextureData(data);
}

Texture::Texture(const std::filesystem::path& path)
{
    assetType = AssetType::texture;
    this->path = path;
    if(!loadFromFile(path))
    {
        m_width = 1;
        m_height = 1;
        loadDebugTexture(GL_RGBA, GL_RGBA8, 1, 1);
    }
    initTexture();
    setTextureData(data);
}

Texture::~Texture()
{
    if(data)
        free(data);
}

void Texture::doReload()
{
    GLenum oldFormat = m_format;
    if(loadFromFile(path))
    {
        if(oldFormat != m_format)
        {
            ASSERT_ERROR(0, "Error: You cant hot reload a texture using diffirent data format");
            return;
        }
        std::cout<<"Sprite "<<path<<" reloaded.\n";
        setTextureData(data);
        reloadScheduled = false;
    }
}

void Texture::loadDebugTexture(GLenum format, GLenum formatInternal, uint32 width, uint32 height)
{
    m_format = format;
    m_formatInternal = formatInternal;
    data = malloc(sizeof(uvec3)*width*height);
    for(uint32 i=0; i<m_width*m_height; i++)
        ((uvec3*)data)[i] = {247, 90, 148};
}

void Texture::initTexture()
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, m_formatInternal, m_width, m_height);

    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::setTextureData(void *d)
{
    if(data && d!=data)
        free(data);
    data = d;
    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, m_format, GL_UNSIGNED_BYTE, data);
}

bool Texture::loadFromFile(const std::filesystem::path& path){

    //TODO: add error logging
    int w, h, ch;
    stbi_set_flip_vertically_on_load(1);

    //Check if the file is still there
    if(!std::filesystem::exists(path))
        return false;

    //Delete old data
    if(data != nullptr){
        free(data);
        data = nullptr;
    }

    //Try to load file
    // we should log this? it might happen often
    data = (void*)stbi_load(path.string().c_str(), &w, &h, &ch, 0);
    if(!data)
        return false;

    switch (ch) {
    case 3:
        m_formatInternal = GL_RGB8;
        m_format = GL_RGB;
        break;
    case 4:
        m_formatInternal = GL_RGBA8;
        m_format = GL_RGBA;
        break;
    default:
        ASSERT_ERROR(0, "Error: Image format not supported!");
        return false;
    }
    m_width = w;
    m_height = h;

    return true;
}


ShaderFile::ShaderFile(const std::filesystem::path &path, const std::string shaderName)
    : m_shaderName(shaderName)
{
    assetType = AssetType::shaderFile;
    this->path = path;
    data = loadFile();

    if(data)
        if(!getTypeFromFile())
        {
            free(data);
            data = nullptr;
        }

}

ShaderFile::ShaderFile(const std::filesystem::path &path, ShaderFile::ShaderType type, const std::string shaderName)
    : m_shaderName(shaderName), type(type)
{
    assetType = AssetType::shaderFile;
    this->path = path;
    data = loadFile();
}

void ShaderFile::doReload()
{
    char* temp = loadFile();
    if(temp)
    {
        if(data)
            free(data);
        data = temp;
        reloadScheduled = false;
        std::cout<<"Shader "<<path<<" reloaded.\n";
    }
}

char* ShaderFile::loadFile()
{
    if(!std::filesystem::exists(path))
    {
        ASSERT_WARNING(0, "Failed to load shader: File %s doesnt exist.", path.string().c_str());
        return nullptr;
    }
    FILE* file;
    if(_wfopen_s(&file, path.c_str(), L"r"))
    {
        ASSERT_WARNING(0, "Failed to load shader: Couldnt open file %s for reading.", path.string().c_str());
        return nullptr;
    }
    fseek(file, 0, SEEK_END);
    uint64 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint64 offset = ftell(file);
    size-=offset;
    // size of the file will be bigger than number of chars becouse of the windows \r\n bullshit
    // the solution is to just calloc the memory so we have zero termination in the right place anyway
    char* str = (char*)calloc(size+1, sizeof (char));
    fread(str, 1, size, file);
    fclose(file);
    str[size] = 0;
    for(uint64 i=0; i<size+1; i++)
        std::cout<<str[i];
    return str;
}

bool ShaderFile::getTypeFromFile()
{
    char* token = strstr (data,"//type");
    if(!token)
    {
        ASSERT_WARNING(0, "Failed to load shader: Couldnt find type token in file %s.", path.string().c_str());
        return false;
    }
    char shaderType[50];

    int index = 0;
    token+=6;
    while(isspace(*token)) token++;
    while(*token != '\n')
    {
        while(isspace(*token)) token++;
        if(index >= 50)
        {
            ASSERT_WARNING(0, "Failed to load shader: Cant have spaces or other characters on the line with shader token");
            return false;
        }
        shaderType[index] = *token;
        token++;
        index++;
    }
    shaderType[index] = 0;

    if(strcmp(shaderType, "vertex") == 0)
        type = vertex;
    else if(strcmp(shaderType, "tessControl") == 0)
        type = tessControl;
    else if(strcmp(shaderType, "tessEval") == 0)
        type = tessEval;
    else if(strcmp(shaderType, "geometry") == 0)
        type = geometry;
    else if(strcmp(shaderType, "fragment") == 0)
        type = fragment;
    else if(strcmp(shaderType, "compute") == 0)
        type = compute;
    else
    {
        ASSERT_WARNING(0, "Failed to load shader: Couldnt match token %s to shader type.", shaderType);
        return false;
    }
    return true;
}
