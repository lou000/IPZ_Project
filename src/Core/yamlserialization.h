#pragma once
#include "yaml-cpp/yaml.h"
#include <string>
#include <filesystem>

struct RenderConfig;

bool serializeRenderConfig(const RenderConfig &config, const std::filesystem::path &filepath);
RenderConfig deserializeRenderConfig(const std::filesystem::path &filepath);
