#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "asset_manager.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_set>
#include <cstdlib> //calloc
#include "../Renderer/buffer.h" // VAO


Texture::Texture(uint width, uint height, uint depth, GLenum formatInternal, uint samples, bool loadDebug)
    : m_width(width), m_height(height), m_depth(depth), m_samples(samples), m_formatInternal(formatInternal)
{
    ASSERT(width*height*depth>0); // none of the dimensions can be zero
    assetType = AssetType::texture;
    initTexture();
    if(loadDebug)
        clear({0.969f, 0.353f, 0.580f});
}

Texture::Texture(const std::filesystem::path& path, uint samples)
    :m_samples(samples)
{
    //for now we dont load 3d images from files
    m_depth  = 1;
    m_width  = 1;
    m_height = 1;

    assetType = AssetType::texture;
    auto data = loadFromFile(path);
    initTexture();
    if(!data)
    {
        clear({0.969f, 0.353f, 0.580f});
        return;
    }
    setTextureData(data, getSize());
    free(data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

bool Texture::doReload()
{
    GLenum oldFormat = m_formatInternal;
    uint oldWidth  = m_width;
    uint oldHeight = m_height;

    auto data = loadFromFile(path);
    if(!data) return false;

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
    free(data);
    return true;
}

void Texture::clear(vec3 color)
{
    auto format = textureSizedFormatToFormat(m_formatInternal);
    glClearTexImage(m_id, 0, format, GL_FLOAT, &color[0]);
}

void Texture::initTexture()
{

    auto format = textureSizedFormatToFormat(m_formatInternal);
    auto type = GL_UNSIGNED_BYTE;
    if(m_formatInternal == GL_DEPTH24_STENCIL8)
        type = GL_UNSIGNED_INT_24_8;
    if(m_depth == 1)
    {
        if(m_samples>1)
        {
            //this is only for framebuffer for now
            glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_id);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, m_formatInternal, m_width, m_height, GL_FALSE);
        }
        else{
            glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
            glBindTexture(GL_TEXTURE_2D, m_id);
            glTexImage2D(GL_TEXTURE_2D, 0, m_formatInternal, m_width, m_height, 0, format, type, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
    }
    else
    {
        if(m_samples>1)
        {
            //this is only for framebuffer for now
            glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE_ARRAY , 1, &m_id);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_id);
            glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY , m_samples, m_formatInternal, m_width, m_height, m_depth, GL_FALSE);
        }
        else{
            glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_id);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_id);
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, m_formatInternal, m_width, m_height, m_depth, 0, format, type, nullptr);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
    }
}

void Texture::setTextureData(void *d, size_t size)
{
    auto s = getSize();
    ASSERT(size<=s);
    if(m_depth == 1)
        glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, textureSizedFormatToFormat(m_formatInternal), GL_UNSIGNED_BYTE, d);
    else
        glTextureSubImage3D(m_id, 0, 0, 0, 0, m_width, m_height, m_depth, textureSizedFormatToFormat(m_formatInternal), GL_UNSIGNED_BYTE, d);
}

size_t Texture::getSize()
{
    //FIXME: will not work for formats other then GL_RGB and GL_RGBA
    size_t dataSize = m_formatInternal == GL_RGB8 ? sizeof(stbi_uc)*3 : sizeof(stbi_uc)*4;
    return dataSize*m_width*m_height*m_depth;
}

void Texture::resize(vec3 size)
{
    m_width  = size.x;
    m_height = size.y;
    m_depth  = size.z;
    ASSERT(m_width*m_height*m_depth>0); // none of the dimensions can be zero

    auto format = textureSizedFormatToFormat(m_formatInternal);
    auto type = GL_UNSIGNED_BYTE;
    if(m_formatInternal == GL_DEPTH24_STENCIL8)
        type = GL_UNSIGNED_INT_24_8;

    if(m_depth == 1)
    {
        if(m_samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, m_formatInternal, m_width, m_height, GL_FALSE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, m_id);
            glTexImage2D(GL_TEXTURE_2D, 0, m_formatInternal, m_width, m_height, 0, format, type, nullptr);
        }
    }
    else
    {
        if(m_samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_id);
            glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY , m_samples, m_formatInternal, m_width, m_height, m_depth, GL_FALSE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_id);
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, m_formatInternal, m_width, m_height, m_depth, 0, format, type, nullptr);
        }
    }
}

vec3 Texture::getDimensions()
{
    return vec3(m_width, m_height, m_depth);
}

void Texture::bind(uint slot)
{
    glBindTextureUnit(slot, m_id);
}

void Texture::selectLayerForNextDraw(uint layer)
{
    m_selectedLayer = layer;
}

void* Texture::loadFromFile(const std::filesystem::path& path){

    //TODO: add error logging
    int w, h, ch;
    stbi_set_flip_vertically_on_load(1);

    //Check if the file is still there
    if(!std::filesystem::exists(path))
        return nullptr;

    //Try to load file
    // we should log this? it might happen often
    auto data = (void*)stbi_load(path.string().c_str(), &w, &h, &ch, 0);
    if(!data)
        return nullptr;

    switch (ch) {
    case 3:
        m_formatInternal = GL_RGB8;
        break;
    case 4:
        m_formatInternal = GL_RGBA8;
        break;
    default:
        ASSERT(0, "Error: Image format not supported!");
        return nullptr;
    }
    m_width = w;
    m_height = h;

    return data;
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
    const aiScene* scene = importer->ReadFile( str, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_GenBoundingBoxes);
    if(!scene)
    {
        WARN("Asset: Couldnt load asset %s, assimp failed with error %s.", c_str, importer->GetErrorString());
        return false;
    }

    // For now consider only one mesh and only vertices/indices.
    // Materials, textures and scenes will come later
    std::vector<uint16> indices;
    std::vector<float> vertices; // resize

    AABB modelBB;
    LOG("\nLoading asset %s\n", c_str);
    for(uint s=0; s<scene->mNumMeshes; s++)
    {
        Material material;
        auto mesh = scene->mMeshes[s];
        auto mat = scene->mMaterials[mesh->mMaterialIndex];

        uint vertexSize = sizeof(MeshVertex)/sizeof(float); //in floats
        vertices.resize(vertexSize*mesh->mNumVertices);
        indices.clear();


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

        if(AI_SUCCESS != mat->Get(AI_MATKEY_SHININESS, material.roughness))
        {
            WARN("Couldnt read material roughness of asset %s", c_str);
        }
        else
        {
            material.roughness = (1-material.roughness/100); //bring to <-1, 1> and invert
            material.roughness = glm::abs(1-glm::sqrt(1.f-material.roughness));
        }

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
        LOG("Mesh %s\n v: %d  i: %d\nRoughness: %f  Metallness: %f\n", mesh->mName.C_Str(),
            (int)vertices.size()/vertexSize, (int)indices.size(), material.roughness, material.metallic);
        AABB bb;
        memcpy(&bb.min, &mesh->mAABB.mMin, sizeof (vec3));
        memcpy(&bb.max, &mesh->mAABB.mMax, sizeof (vec3));

        // correct mesh AABB
        if(modelBB.max.x<bb.max.x)
            modelBB.max.x = bb.max.x;
        if(modelBB.max.y<bb.max.y)
            modelBB.max.y = bb.max.y;
        if(modelBB.max.z<bb.max.z)
            modelBB.max.z = bb.max.z;

        if(modelBB.min.x>bb.min.x)
            modelBB.min.x = bb.min.x;
        if(modelBB.min.y>bb.min.y)
            modelBB.min.y = bb.min.y;
        if(modelBB.min.z>bb.min.z)
            modelBB.min.z = bb.min.z;

        m_meshes.push_back(std::make_shared<Mesh>(vertices.data(), vertices.size()/vertexSize,
                                                  indices.data(), indices.size(), material, bb));
    }
    LOG("\n");
    m_boundingBox = modelBB;

    return true;
}

