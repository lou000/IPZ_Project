#pragma once
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include <string>
#include <filesystem>

struct RenderConfig;
struct DirectionalLight;
class Scene;
class Camera;
class Entity;



class Serializer
{
public:
    static bool serializeRenderConfig(const RenderConfig &config, const std::filesystem::path &filepath);
    static RenderConfig deserializeRenderConfig(const std::filesystem::path &filepath);

    static bool serializeDirLight(YAML::Emitter &e, DirectionalLight dirLight);
    static DirectionalLight deserializeDirLight(const YAML::Node &node);

    static bool serializeCamera(YAML::Emitter& e, std::shared_ptr<Camera> camera);
    static std::shared_ptr<Camera> deserializeCamera(const YAML::Node& node);

    static bool serializeEntity(YAML::Emitter& e, const std::shared_ptr<Entity> &entity);
    static std::shared_ptr<Entity> deserializeEntity(const YAML::Node& node);

    static bool serializeScene(Scene* scene, const std::filesystem::path &filepath);
    static bool deserializeScene(Scene* scene, const std::filesystem::path &filepath);

};
