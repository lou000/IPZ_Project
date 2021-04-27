#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"
#include <unordered_set>
#include <cstdlib> //calloc
#include "../Renderer/buffer.h" // VAO

Texture::Texture(uint width, uint height, GLenum formatInternal, uint samples, bool loadDebug)
    : m_width(width), m_height(height), m_samples(samples), m_formatInternal(formatInternal)
{
    assetType = AssetType::texture;
    initTexture();
    if(loadDebug)
    {
        loadDebugTexture(formatInternal, width, height);
        setTextureData(data, getSize());
    }
}

Texture::Texture(const std::filesystem::path& path, uint samples)
    :m_samples(samples)
{
    assetType = AssetType::texture;
    this->path = path;
    if(!loadFromFile(path))
    {
        m_width = 1;
        m_height = 1;
        loadDebugTexture(GL_RGBA, 1, 1);
    }
    initTexture();
    setTextureData(data, getSize());
}

Texture::~Texture()
{
    if(data)
        free(data);
    glDeleteTextures(1, &m_id);
}

bool Texture::doReload()
{
    GLenum oldFormat = m_formatInternal;
    uint oldWidth  = m_width;
    uint oldHeight = m_height;
    if(loadFromFile(path))
    {
        if(oldFormat != m_formatInternal)
        {
            WARN("AssetManager: Reloading texture with a diffirent format, texture storage will be recreated");
            glDeleteTextures(1, &m_id);
            initTexture();
        }else if(oldWidth != m_width || oldHeight != m_height)
        {
            WARN("AssetManager: Reloading texture with a diffirent size, texture storage will be recreated");
            glDeleteTextures(1, &m_id);
            initTexture();
        }
        setTextureData(data, getSize());
        return true;
    }
    return false;
}

void Texture::loadDebugTexture(GLenum formatInternal, uint width, uint height)
{
    m_formatInternal = formatInternal;
    data = malloc(sizeof(uvec4)*width*height);
    for(uint i=0; i<m_width*m_height; i++)
        ((uvec4*)data)[i] = {247, 90, 148, 255};
}

void Texture::initTexture()
{
    if(m_samples>1)
    {
        //this is only for framebuffer for now
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, m_formatInternal, m_width, m_height, GL_FALSE);
        glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_id);
    }
    else{
        glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
        glTextureStorage2D(m_id, 1, m_formatInternal, m_width, m_height);
        glGenerateTextureMipmap(m_id);
        glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GLfloat maxAnisotropy = 0.f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        glTextureParameterf(m_id, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

}

void Texture::setTextureData(void *d, size_t size)
{
    if(data != d || data == nullptr)
    {
        if(data)
            free(data);
        data = malloc(size); //allocate our own memory becouse someone could have given us a pointer thats about to expire
        memcpy_s(data, size, d, size);
    }
    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, textureSizedFormatToFormat(m_formatInternal), GL_UNSIGNED_BYTE, data);
}

size_t Texture::getSize()
{
    //FIXME: will not work for formats other then GL_RGB and GL_RGBA
    size_t dataSize = m_formatInternal == GL_RGB8 ? sizeof(stbi_uc)*3 : sizeof(stbi_uc)*4;
    return dataSize*m_width*m_height;
}

void Texture::bind(uint slot)
{
    glBindTextureUnit(slot, m_id);
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
        break;
    case 4:
        m_formatInternal = GL_RGBA8;
        break;
    default:
        ASSERT(0, "Error: Image format not supported!");
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

bool ShaderFile::doReload()
{
    char* temp = loadFile();
    if(temp)
    {
        if(data)
            free(data);
        data = temp;
        return true;
    }
    return false;
}

char* ShaderFile::loadFile()
{
    if(!std::filesystem::exists(path))
    {
        WARN("Failed to load shader: File %s doesnt exist.", path.string().c_str());
        return nullptr;
    }
    FILE* file;
    if(_wfopen_s(&file, path.c_str(), L"r"))
    {
        WARN("Failed to load shader: Couldnt open file %s for reading.", path.string().c_str());
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
//    for(uint64 i=0; i<size+1; i++)
//        std::cout<<str[i];
    return str;
}

bool ShaderFile::getTypeFromFile()
{
    char* token = strstr (data,"//type");
    if(!token)
    {
        WARN("Failed to load shader: Couldnt find type token in file %s.", path.string().c_str());
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
            WARN("Failed to load shader: Cant have spaces or other characters on the line with shader token");
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
        WARN("Failed to load shader: Couldnt match token %s to shader type.", shaderType);
        return false;
    }
    return true;
}

MeshFile::MeshFile(const std::filesystem::path &path)
{
    this->path = path;
    loadOBJ();
}

bool MeshFile::doReload()
{
    return loadOBJ();
}

void MeshFile::createVAO()
{
    ASSERT(m_vertexData && m_indexData);
    auto iBuffer = std::make_shared<IndexBuffer>(m_indexCount, m_indexData);
    BufferLayout layout = {
        {Shader::Float3, "a_Position" },
        {Shader::Float3, "a_Normal"   },
        {Shader::Float2, "a_TexCoords"}
    };
    auto vBuffer = {std::make_shared<VertexBuffer>(layout, m_vertexCount, m_vertexData)};
    m_vertexArray = std::make_shared<VertexArray>(vBuffer, iBuffer);
}

bool MeshFile::loadOBJ()
{
    auto str = path.string();
    auto c_str = str.c_str();
    if(!std::filesystem::exists(path))
    {
        WARN("Asset: Couldnt load the mesh file %s doesnt exist", c_str);
        return false;
    }

    int componentCount = 8;
    fastObjMesh* mesh = fast_obj_read(c_str);
    if(mesh->position_count <= 1)
    {
        WARN("Asset: Couldnt load mesh %s, there is no vertex pos data.", c_str);
        return false;
    }
    m_stride = componentCount*sizeof(float);

    // We have to convert from obj indices to opengl indices,
    // we can only index full vertices not individual components
    std::unordered_map<size_t, uint16> map;
    std::vector<uint16> indices;
    std::vector<float> vertices;
    int count = 0;
    int globalCount = 0;
    for(uint i=0; i<mesh->face_count; i++)
    {
        for(uint j=0; j<mesh->face_vertices[i]; j++)
        {
            auto index = mesh->indices[globalCount];
            globalCount++;
            size_t hash = 0;
            hash |= ((size_t)index.n)<<16;
            hash |= ((size_t)index.p)<<32;
            hash |= ((size_t)index.t)<<48;

            auto pair = map.insert({hash, count});
            if(pair.second) // if this is a unique vertex
            {
                // push 3 positions
                int indx = index.p*3;
                vertices.push_back(mesh->positions[indx+0]);
                vertices.push_back(mesh->positions[indx+1]);
                vertices.push_back(mesh->positions[indx+2]);

                indx = index.n*3;
                if(mesh->normal_count>1)
                {
                    //push 3 normals
                    vertices.push_back(mesh->normals[indx+0]);
                    vertices.push_back(mesh->normals[indx+1]);
                    vertices.push_back(mesh->normals[indx+2]);
                }
                else
                {
                    vertices.push_back(0);
                    vertices.push_back(0);
                    vertices.push_back(0);
                }

                indx = index.t*2;
                if(mesh->texcoord_count>1)
                {
                    //push 2 texture coordinates
                    vertices.push_back(mesh->texcoords[indx+0]);
                    vertices.push_back(mesh->texcoords[indx+1]);
                }
                else
                {
                    vertices.push_back(0);
                    vertices.push_back(0);
                }

                indices.push_back(count);//push index
                count++;
            }
            else
            {
                ASSERT(hash != (*pair.first).second);
                indices.push_back((*pair.first).second);//push index of previous vertex
            }
        }
    }
    delete mesh;
    // copy vectors memory to storage

    if(m_vertexData)
        free(m_vertexData);
    if(m_indexData)
        free(m_indexData);

    m_vertexData = (float*)malloc(vertices.size()*sizeof(float));
    memcpy(m_vertexData, vertices.data(), vertices.size()*sizeof(float));
    m_indexData = (uint16*)malloc(indices.size()*sizeof(uint16));
    memcpy(m_indexData, indices.data(), indices.size()*sizeof(uint16));
    m_indexCount = (uint)indices.size();
    m_vertexCount = (uint)vertices.size()/(m_stride/sizeof(float));
    return true;
}
