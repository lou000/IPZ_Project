#include "yamlserialization.h"
#include "../Renderer/renderpipeline.h"
#include <fstream>

#define SERIALIZE_VALUE(e, k) \
    e << Key << #k << Value << k;

#define DESERIALIZE_IF_PRESENT(node, dest, type)\
do{                                             \
        auto maybe = node[#dest];               \
        if(maybe)                               \
            dest = maybe.as<type>();            \
} while(0);

using namespace YAML;

namespace YAML {
template<>
struct convert<glm::vec3>
{
    static Node encode(const glm::vec3& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.SetStyle(EmitterStyle::Flow);
        return node;
    }

    static bool decode(const Node& node, glm::vec3& rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};

template<>
struct convert<glm::vec4>
{
    static Node encode(const glm::vec4& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        node.SetStyle(EmitterStyle::Flow);
        return node;
    }

    static bool decode(const Node& node, glm::vec4& rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};

template<>
struct convert<glm::quat>
{
    static Node encode(const glm::quat& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        node.SetStyle(EmitterStyle::Flow);
        return node;
    }

    static bool decode(const Node& node, glm::quat& rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};

Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

}

bool readFile(std::string* strOut, const std::filesystem::path& path)
{
    if(!std::filesystem::exists(path))
    {
        WARN("Config: File %s doesnt exist.", path.string().c_str());
        return false;
    }

    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        WARN("Config: Couldnt open %s file. Config will revert to default!",
             path.string().c_str());
        return false;
    }
    std::stringstream strStream;
    strStream << input_file.rdbuf();
    *strOut = strStream.str();
    input_file.close();
    return true;
}

bool writeFile(const char* str, const std::filesystem::path& path)
{
    std::ofstream fout(path);
    if (!fout.is_open()) {
        WARN("Config: Couldnt open %s file. Config will not be saved!",
             path.string().c_str());
        return false;
    }
    fout << str;
    fout.close();
    return true;
}

bool Serializer::serializeRenderConfig(const RenderConfig &config, const std::filesystem::path &filepath)
{
    LOG("Config: Serializing render pipeline config to file %s\n",
        filepath.string().c_str());
    Emitter e;
    e << BeginMap;
    SERIALIZE_VALUE(e, config.enableBloom);
    SERIALIZE_VALUE(e, config.bloomRadius);
    SERIALIZE_VALUE(e, config.bloomIntensity);
    SERIALIZE_VALUE(e, config.bloomTreshold);
    SERIALIZE_VALUE(e, config.exposure);

    SERIALIZE_VALUE(e, config.enableCSM);
    SERIALIZE_VALUE(e, config.shadowCascadeCount);
    SERIALIZE_VALUE(e, config.cascadeZextra);
    SERIALIZE_VALUE(e, config.firstCascadeOffset);
    SERIALIZE_VALUE(e, config.csmResolution);

    SERIALIZE_VALUE(e, config.enableSSAO);
    SERIALIZE_VALUE(e, config.ssaoKernelSize);
    SERIALIZE_VALUE(e, config.blurKernelSize);
    SERIALIZE_VALUE(e, config.ssaoRadius);
    SERIALIZE_VALUE(e, config.ssaoBias);
    e << EndMap;
    return writeFile(e.c_str(), filepath);
}

RenderConfig Serializer::deserializeRenderConfig(const std::filesystem::path &filepath)
{
    RenderConfig config;
    std::string in;
    if(!readFile(&in, filepath))
        return config;

    Node data = Load(in);
    DESERIALIZE_IF_PRESENT(data, config.enableBloom,    bool);
    DESERIALIZE_IF_PRESENT(data, config.bloomRadius,    float);
    DESERIALIZE_IF_PRESENT(data, config.bloomIntensity, float);
    DESERIALIZE_IF_PRESENT(data, config.bloomTreshold,  float);
    DESERIALIZE_IF_PRESENT(data, config.exposure,       float);

    DESERIALIZE_IF_PRESENT(data, config.enableCSM,          bool);
    DESERIALIZE_IF_PRESENT(data, config.shadowCascadeCount, int);
    DESERIALIZE_IF_PRESENT(data, config.cascadeZextra,      float);
    DESERIALIZE_IF_PRESENT(data, config.firstCascadeOffset, float);
    DESERIALIZE_IF_PRESENT(data, config.csmResolution,      uint);

    DESERIALIZE_IF_PRESENT(data, config.enableSSAO,     bool);
    DESERIALIZE_IF_PRESENT(data, config.ssaoKernelSize, int);
    DESERIALIZE_IF_PRESENT(data, config.blurKernelSize, int);
    DESERIALIZE_IF_PRESENT(data, config.ssaoRadius,     float);
    DESERIALIZE_IF_PRESENT(data, config.ssaoBias,       float);

    return config;
}

bool Serializer::serializeCamera(Emitter& e, std::shared_ptr<Camera> camera)
{
    e << BeginMap;
    SERIALIZE_VALUE(e, camera->type);
    SERIALIZE_VALUE(e, camera->m_pos);
    SERIALIZE_VALUE(e, camera->m_fov);
    SERIALIZE_VALUE(e, camera->m_aspectRatio);
    SERIALIZE_VALUE(e, camera->m_nearClip);
    SERIALIZE_VALUE(e, camera->m_farClip);
    SERIALIZE_VALUE(e, camera->isActive);
    SERIALIZE_VALUE(e, camera->m_focusPoint);
    SERIALIZE_VALUE(e, camera->m_rotation);
    e << EndMap;
    return false;
}

std::shared_ptr<Camera> Serializer::deserializeCamera(const Node& node)
{
    ASSERT(node["camera->type"]);
    Camera::Type type = (Camera::Type)node["camera->type"].as<uint>();

    float fov = node["camera->m_fov"].as<float>();
    float aspectRatio = node["camera->m_aspectRatio"].as<float>();
    float nearClip = node["camera->m_nearClip"].as<float>();
    float farClip = node["camera->m_farClip"].as<float>();

    std::shared_ptr<Camera> camera;
    switch(type)
    {
    case Camera::Type::Base:
        camera = std::make_shared<Camera>(fov, aspectRatio, nearClip, farClip);
    case Camera::Type::EditorCamera:
        camera = std::make_shared<Camera>(fov, aspectRatio, nearClip, farClip); //TODO: fix after implementing
    case Camera::Type::GameCamera:
        camera = std::make_shared<Camera>(fov, aspectRatio, nearClip, farClip); //TODO: fix after implementing
    }

    DESERIALIZE_IF_PRESENT(node, camera->m_pos, vec3);
    DESERIALIZE_IF_PRESENT(node, camera->isActive, bool);
    DESERIALIZE_IF_PRESENT(node, camera->m_focusPoint, vec3);
    DESERIALIZE_IF_PRESENT(node, camera->m_rotation, quat);

    return std::static_pointer_cast<Camera>(camera);
}

bool Serializer::serializeDirLight(Emitter& e, DirectionalLight dirLight)
{
    e << BeginMap;
    SERIALIZE_VALUE(e, dirLight.direction);
    SERIALIZE_VALUE(e, dirLight.color);
    SERIALIZE_VALUE(e, dirLight.enabled);
    SERIALIZE_VALUE(e, dirLight.intensity);
    e << EndMap;
    return false;
}

DirectionalLight Serializer::deserializeDirLight(const Node& node)
{
    DirectionalLight dirLight;
    DESERIALIZE_IF_PRESENT(node, dirLight.direction, vec3);
    DESERIALIZE_IF_PRESENT(node, dirLight.color,     vec3);
    DESERIALIZE_IF_PRESENT(node, dirLight.enabled,   bool);
    DESERIALIZE_IF_PRESENT(node, dirLight.intensity, float);
    return dirLight;
}

bool Serializer::serializeScene(std::shared_ptr<Scene> scene, const std::filesystem::path &filepath)
{
    //TODO: serialize entities!
    LOG("Config: Serializing scene to file %s\n",
        filepath.string().c_str());
    Emitter e;
    e << BeginMap;
    e << Key << "directionalLight"<< Value;
    serializeDirLight(e, scene->directionalLight);

    e << Key << "m_activeCamera"<<Value << (scene->m_activeCamera == scene->m_editorCamera);

    e << Key << "m_gameCamera"<<Value;
    serializeCamera(e, scene->m_gameCamera);

    e << Key << "m_editorCamera"<<Value;
    serializeCamera(e, scene->m_editorCamera);
    e << EndMap;
    return writeFile(e.c_str(), filepath);
    return false;
}

void Serializer::deserializeScene(std::shared_ptr<Scene> scene, const std::filesystem::path &filepath)
{
    std::string in;
    readFile(&in, filepath);

    Node node = Load(in);
    scene->directionalLight = deserializeDirLight(node["directionalLight"]);
    scene->m_gameCamera = deserializeCamera(node["m_gameCamera"]);
    scene->m_editorCamera = deserializeCamera(node["m_editorCamera"]);
    if(node["m_activeCamera"].as<bool>())
        scene->m_activeCamera = scene->m_editorCamera;
}

