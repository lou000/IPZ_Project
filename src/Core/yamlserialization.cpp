#define YAML_CPP_STATIC_DEFINE
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

bool serializeRenderConfig(const RenderConfig &config, const std::filesystem::path &filepath)
{
    LOG("Config: Serializing render pipeline to file %s\n", filepath.string().c_str());
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

RenderConfig deserializeRenderConfig(const std::filesystem::path &filepath)
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
