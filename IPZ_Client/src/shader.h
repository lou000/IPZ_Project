#pragma once
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <glm.hpp>
#include <memory>
#include "assets.h"
#include "gl.h"

using namespace glm;
class Shader
{
    friend class ShaderFile;
    friend class AssetManager;
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
    ~Shader();

    void bind();
    void unbind();
    uint32 id() {return m_id;}

private:
    uint32 m_id = 0;
    std::string name;
    std::vector<std::shared_ptr<ShaderFile>> files;

private:
    void loadFiles(std::vector<std::filesystem::path> filePaths);
    void compile();
};

