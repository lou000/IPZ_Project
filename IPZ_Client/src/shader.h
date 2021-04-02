#pragma once
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <glm.hpp>
#include <memory>
#include "asset_manager.h"
#include "gl.h"

using namespace glm;
class Shader
{
    enum UniformType{
        _int,
        _float,
        vecf2,
        vecf3,
        vecf4,
        mat3,
        mat4,
    };

public:
    Shader(const std::string& name, std::vector<std::filesystem::path> filePaths);

private:
    uint32 id;
    std::string name;
    std::vector<std::shared_ptr<ShaderFile>> files;

private:
    void loadFiles(std::vector<std::filesystem::path> filePaths);
    void compile();
};

