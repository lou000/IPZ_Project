#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(GLenum format, GLenum formatInternal, uint32 width, uint32 height)
    : m_width(width), m_height(height), m_format(format), m_formatInternal(formatInternal)
{
    initTexture();
    loadDebugTexture(format, formatInternal, width, height);
    setTextureData(data);
}

Texture::Texture(const std::filesystem::path& path)
{
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
        if(oldFormat == m_format)
        {
            ASSERT_ERROR(oldFormat == m_format, "Error: You cant hot reload a texture with diffirent data format");
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
    if(data)
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
