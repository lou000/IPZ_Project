#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "asset_manager.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
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

Model::Model(const std::filesystem::path &path)
{
    this->path = path;
    loadModel();
}

bool Model::doReload()
{
    return loadModel();
}

bool Model::loadModel()
{
    auto str = path.string();
    auto c_str = str.c_str();
    UNUSED(c_str);
    if(!std::filesystem::exists(path))
    {
        WARN("Asset: Couldnt load the asset file %s doesnt exist", c_str);
        return false;
    }


    auto importer = AssetManager::getAssimpImporter();
    importer->SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 1);
    const aiScene* scene = importer->ReadFile( str, aiProcessPreset_TargetRealtime_Quality);
    if(!scene)
    {
        WARN("Asset: Couldnt load asset %s, assimp failed with error %s.", c_str, importer->GetErrorString());
        return false;
    }

    // For now consider only one mesh and only vertices/indices.
    // Materials, textures and scenes will come later
    std::vector<uint16> indices;
    std::vector<float> vertices; // resize

    // TODO: deal with materials and textures

    for(uint i=0; i<scene->mNumMeshes; i++)
    {
        Material material;
        auto mesh = scene->mMeshes[i];
        auto mat = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D matColor;
        if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_DIFFUSE, matColor))
        {
            WARN("Couldnt read material color of asset %s", c_str);
        }
        else
        {
            material.color = {matColor.r, matColor.g, matColor.b, 1};
        }

        if(AI_SUCCESS != mat->Get(AI_MATKEY_REFLECTIVITY, material.metallic))
        {
            WARN("Couldnt read material metallnes of asset %s", c_str);
        }
        else
        {
            LOG("Metallnes of %s is %f", c_str, material.metallic);
        }

        if(AI_SUCCESS != mat->Get(AI_MATKEY_SHININESS, material.roughness))
        {
            WARN("Couldnt read material roughness of asset %s", c_str);
        }
        else
        {
            material.roughness = (1-material.roughness/100); //bring to <-1, 1> and invert
            material.roughness = glm::abs(1-glm::sqrt(1.f-material.roughness));
            LOG("Roughness of %s is %f", c_str, material.roughness);
        }


        uint vertexSize = sizeof(MeshVertex)/sizeof(float); //in floats

        vertices.resize(vertexSize*mesh->mNumVertices);
        for(uint i=0; i<mesh->mNumVertices; i++)
        {
            auto offset = i*vertexSize;
            vertices[offset+0] = mesh->mVertices[i].x;
            vertices[offset+1] = mesh->mVertices[i].y;
            vertices[offset+2] = mesh->mVertices[i].z;

            vertices[offset+3] = mesh->mNormals[i].x;
            vertices[offset+4] = mesh->mNormals[i].y;
            vertices[offset+5] = mesh->mNormals[i].z;

            // we support only one texture per vertex
            if(mesh->mTextureCoords[0])
            {
                vertices[offset+6] = mesh->mTextureCoords[0][i].x;
                vertices[offset+7] = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertices[offset+6] = 0;
                vertices[offset+7] = 0;
            }
        }

        for(uint i=0; i<mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(uint j=0; j<face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        m_meshes.push_back(std::make_shared<Mesh>(vertices.data(), vertices.size()/vertexSize,
                                                  indices.data(), indices.size(), material));
    }

    return true;
}

