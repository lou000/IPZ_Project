#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "asset_manager.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include "../Renderer/buffer.h" // VAO
#define DR_WAV_IMPLEMENTATION
#include<dr_wav.h>

std::string Asset::getName()
{
    return m_path.empty() ? m_name : m_path.string();
}

AudioBuffer::AudioBuffer(const std::filesystem::path &path)
    :Asset(AssetType::audioBuffer)
{
    init();
    m_path = path;
    auto data = loadFromFile();
    if(!data)
        return;
    m_ALFormat = m_channels>1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    setData(data, m_framecount * m_channels * sizeof(uint16), m_ALFormat, m_samplerate);
    free(data);
}

AudioBuffer::~AudioBuffer()
{
    alDeleteBuffers(1, &this->m_bufferID);
}

bool AudioBuffer::doReload()
{
    auto data = loadFromFile();
    if(!data)
        return false;
    m_ALFormat = m_channels>1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    setData(data, m_framecount * m_channels * sizeof(uint16), m_ALFormat, m_samplerate);
    free(data);
    return true;
}

void AudioBuffer::init()
{
    alGenBuffers(1, &this->m_bufferID);
}

void AudioBuffer::setData(void *data, size_t size, ALenum format, uint samplerate)
{
    ASSERT(m_bufferID);
    alBufferData(m_bufferID, format, data, (ALsizei)size, samplerate);
}

void* AudioBuffer::loadFromFile()
{
    auto path = m_path.string();
    drwav_int16* const pSampleData =
        drwav_open_file_and_read_pcm_frames_s16(path.c_str(), &m_channels,
                                                &m_samplerate, &m_framecount, nullptr);
    if (!pSampleData) {
        drwav_free(pSampleData, nullptr);
        WARN("Audio: Failed to load file %s", path.c_str());
        return nullptr;
    }
    if (m_framecount * m_channels > drwav_uint64(std::numeric_limits<size_t>::max()))
    {
        drwav_free(pSampleData, nullptr);
        WARN("Audio: Failed to load file %s, too much data", path.c_str());
        return nullptr;
    }

    return pSampleData;
}

Texture::Texture(uint width, uint height, uint depth, GLenum formatInternal,  GLenum textureWrap, uint samples, bool loadDebug)
    : m_width(width), m_height(height), m_depth(depth), m_samples(samples), m_glFormatSized(formatInternal), m_textureWrap(textureWrap)
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
    GLenum oldFormat = m_glFormatSized;
    uint oldWidth  = m_width;
    uint oldHeight = m_height;

    auto data = loadFromFile(m_path);
    if(!data) return false;

    if(oldFormat != m_glFormatSized)
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
    auto format = textureSizedFormatToFormat(m_glFormatSized);
    glClearTexImage(m_id, 0, format, GL_FLOAT, &color[0]);
}

void Texture::initTexture()
{

    auto format = textureSizedFormatToFormat(m_glFormatSized);
    auto dataType = GL_UNSIGNED_BYTE;

    if(m_glFormatSized == GL_DEPTH24_STENCIL8)
        dataType = GL_UNSIGNED_INT_24_8;
    else if(m_glFormatSized == GL_RGB16F)
        dataType = GL_HALF_FLOAT;

    if(m_depth == 1)
    {
        if(m_samples>1)
        {
            m_glType = GL_TEXTURE_2D_MULTISAMPLE;
            glCreateTextures(m_glType, 1, &m_id);
            glBindTexture(m_glType, m_id);
            glTexImage2DMultisample(m_glType, m_samples, m_glFormatSized, m_width, m_height, GL_FALSE);
        }
        else{
            m_glType = GL_TEXTURE_2D;
            glCreateTextures(m_glType, 1, &m_id);
            glBindTexture(m_glType, m_id);
            glTexImage2D(m_glType, 0, m_glFormatSized, m_width, m_height, 0, format, dataType, nullptr);
            glTexParameteri(m_glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(m_glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(m_glType, GL_TEXTURE_WRAP_R, m_textureWrap);
            glTexParameteri(m_glType, GL_TEXTURE_WRAP_S, m_textureWrap);
            glTexParameteri(m_glType, GL_TEXTURE_WRAP_T, m_textureWrap);
        }
    }
    else
    {
        if(m_samples>1)
        {
            m_glType = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
            glCreateTextures(m_glType , 1, &m_id);
            glBindTexture(m_glType, m_id);
            glTexImage3DMultisample(m_glType , m_samples, m_glFormatSized, m_width, m_height, m_depth, GL_FALSE);
        }
        else{
            m_glType = GL_TEXTURE_2D_ARRAY;
            glCreateTextures(m_glType, 1, &m_id);
            glBindTexture(m_glType, m_id);
            glTexImage3D(m_glType, 0, m_glFormatSized, m_width, m_height, m_depth, 0, format, dataType, nullptr);
            glTexParameteri(m_glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(m_glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(m_glType, GL_TEXTURE_WRAP_R, m_textureWrap);
            glTexParameteri(m_glType, GL_TEXTURE_WRAP_S, m_textureWrap);
            glTexParameteri(m_glType, GL_TEXTURE_WRAP_T, m_textureWrap);
        }
    }
}

void Texture::setTextureData(void *d, size_t size)
{
//    auto s = getSize();
//    ASSERT(size<=s);
    auto dataType = GL_UNSIGNED_BYTE;

    if(m_glFormatSized == GL_DEPTH24_STENCIL8)
        dataType = GL_UNSIGNED_INT_24_8;
    else if(m_glFormatSized == GL_RGB16F)
        dataType = GL_FLOAT;

    if(m_depth == 1)
        glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, textureSizedFormatToFormat(m_glFormatSized), dataType, d);
    else
        glTextureSubImage3D(m_id, 0, 0, 0, 0, m_width, m_height, m_depth, textureSizedFormatToFormat(m_glFormatSized), dataType, d);
}

size_t Texture::getSize()
{
    //FIXME: will not work for formats other then GL_RGB and GL_RGBA
    size_t dataSize = m_glFormatSized == GL_RGB8 ? sizeof(stbi_uc)*3 : sizeof(stbi_uc)*4;
    return dataSize*m_width*m_height*m_depth;
}

void Texture::resize(vec3 size)
{
    m_width  = size.x;
    m_height = size.y;
    m_depth  = size.z;
    ASSERT(m_width*m_height*m_depth>0); // none of the dimensions can be zero

    auto format = textureSizedFormatToFormat(m_glFormatSized);
    auto type = GL_UNSIGNED_BYTE;
    if(m_glFormatSized == GL_DEPTH24_STENCIL8)
        type = GL_UNSIGNED_INT_24_8;

    if(m_depth == 1)
    {
        if(m_samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, m_glFormatSized, m_width, m_height, GL_FALSE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, m_id);
            glTexImage2D(GL_TEXTURE_2D, 0, m_glFormatSized, m_width, m_height, 0, format, type, nullptr);
        }
    }
    else
    {
        if(m_samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_id);
            glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY , m_samples, m_glFormatSized, m_width, m_height, m_depth, GL_FALSE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_id);
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, m_glFormatSized, m_width, m_height, m_depth, 0, format, type, nullptr);
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

void Texture::copyTo(std::shared_ptr<Texture> target, vec3 srcXYZ, vec3 destXYZ, vec3 size)
{
    glCopyImageSubData(this->m_id, this->m_glType,     0, srcXYZ.x,  srcXYZ.y,  srcXYZ.z,
                       target->id(), target->glType(), 0, destXYZ.x, destXYZ.y, destXYZ.z,
                       size.x, size.y, size.z);
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
        m_glFormatSized = GL_RGB8;
        break;
    case 4:
        m_glFormatSized = GL_RGBA8;
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
    : m_programName(shaderName)
{
    assetType = AssetType::shaderFile;
    this->m_path = path;
    if(!getTypeFromFileName()) return;
    text = loadFile();
}

ShaderFile::ShaderFile(const std::filesystem::path &path, ShaderFile::ShaderType type, const std::string shaderName)
    : m_programName(shaderName), type(type)
{
    assetType = AssetType::shaderFile;
    this->m_path = path;
    text = loadFile();
}

bool ShaderFile::doReload()
{
    text = loadFile();
    return false;
}

std::string ShaderFile::loadFile()
{
    if(!std::filesystem::exists(m_path))
    {
        WARN("Failed to load shader: File %s doesnt exist.", m_path.string().c_str());
        return nullptr;
    }

    std::ifstream input_file(m_path);
    if (!input_file.is_open()) {
        WARN("Failed to load shader: Couldnt open file %s for reading.", m_path.string().c_str());
        return nullptr;
    }

    auto str = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

//    std::cout<<str;
    return str;
}

bool ShaderFile::getTypeFromFileName()
{
    auto extension = this->m_path.extension();

    if(extension == ".vs")
        type = vertex;
    else if(extension == ".tc")
        type = tessControl;
    else if(extension == ".tes")
        type = tessEval;
    else if(extension == ".gs")
        type = geometry;
    else if(extension == ".fs")
        type = fragment;
    else if(extension == ".cmp")
        type = compute;
    else
    {
        WARN("Failed to load shader: Couldnt match file extension %ws to shader type.", extension.c_str());
        return false;
    }
    return true;
}

Model::Model(const std::string &name, bool loadDebug)
{
    this->m_name = name;
}

Model::Model(const std::filesystem::path &path)
{
    this->m_path = path;
    loadModel();
}

Model::Model(std::string name, std::vector<std::shared_ptr<Mesh> > meshes)
{
    m_name = name;
    AABB modelBB;
    for(const auto& m : meshes)
    {
        auto bb = m->boundingBox;
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
        m_meshes.push_back(m);
    }
}

bool Model::doReload()
{
    return loadModel();
}

std::shared_ptr<Model> Model::makeUnitPlane()
{
    constexpr vec2 textureCoords[4]={
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    constexpr vec3 vertexPos[4]={
        {-0.5f,  0.0f,  0.5f},
        { 0.5f,  0.0f,  0.5f},
        { 0.5f,  0.0f, -0.5f},
        {-0.5f,  0.0f, -0.5f}
    };

    constexpr vec3 normals[4]={
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    };

    constexpr uint16 indices[6]={
        0,1,3,3,1,2
    };

    MeshVertex vertices[4];
    for(uint i=0; i<4; i++)
    {
        vertices[i].position  = vertexPos[i];
        vertices[i].normal    = normals[i];
        vertices[i].texCoords = textureCoords[i];
    }
    Material mat;
    mat.metallic = 0;
    mat.roughness = 0.2;
    mat.color = {1,1,1,1};
    auto m = std::make_shared<Mesh>((float*)&vertices[0], 4, (uint16*)&indices[0], 6, mat);

    return std::make_shared<Model>("unitPlane", std::vector<std::shared_ptr<Mesh>>{m});
}

bool Model::loadModel()
{
    auto str = m_path.string();
    auto c_str = str.c_str();
    UNUSED(c_str);
    if(!std::filesystem::exists(m_path))
    {
        WARN("Asset: Couldnt load the asset file %s doesnt exist", c_str);
        return false;
    }

    if(m_path.extension() != ".fbx" && m_path.extension() != ".obj")
    {
        WARN("Asset: Couldnt load the asset file %s extension is not supported.", c_str);
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
    LOG("Loading asset %s\n", c_str);
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

        //Doesnt work with blender fbx
//        aiColor3D emissiveColor;
//        if(AI_SUCCESS != mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor))
//        {
//            WARN("Couldnt read emmisive color of asset %s", c_str);
//        }
//        else
//        {
//            material.colorEmissive = {matColor.r, matColor.g, matColor.b, 1};
//        }

        if(AI_SUCCESS != mat->Get(AI_MATKEY_REFLECTIVITY, material.metallic))
        {
            WARN("Couldnt read material metallnes of asset %s", c_str);
        }
        //Doesnt work with blender fbx
//        if(AI_SUCCESS != mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, material.emissiveIntensity))
//        {
//            WARN("Couldnt read material emmisiveIntensity of asset %s", c_str);
//        }
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
//        LOG("Mesh %s\n v: %d  i: %d\nRoughness: %f  Metallness: %f\n", mesh->mName.C_Str(),
//            (int)vertices.size()/vertexSize, (int)indices.size(), material.roughness, material.metallic);
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
    m_boundingBox = modelBB;

    return true;
}



