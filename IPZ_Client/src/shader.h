#pragma once
#include <gtc/type_ptr.hpp>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <glm.hpp>
#include <memory>
#include <any>
#include "assets.h"
#include "glad/glad.h"

using namespace glm;
class Shader
{
    friend class ShaderFile;
    friend class AssetManager;

public:
    enum DataType{
        Int,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
    };

    Shader(const std::string& name, std::vector<std::filesystem::path> filePaths);
    ~Shader();

    void bind();
    void unbind();
    uint32 id() {return m_id;}
    void setUniform(const char* name, DataType type, const std::any& value, bool transpose = false);
    void setUniformArray(const char* name, DataType type, const std::any& value, uint count, bool transpose = false);

    inline static uint typeComponentCount(DataType type){
        switch (type)
        {
        case Int:    return 1;
        case Float:  return 1;
        case Float2: return 2;
        case Float3: return 3;
        case Float4: return 4;
        case Mat3:   return 3;
        case Mat4:   return 4;
        }
        return 0;
    }
    inline static uint typeComponentSize(DataType type){
        switch (type)
        {
        case Int:    return 4;
        case Float:  return 4;
        case Float2: return 4*2;
        case Float3: return 4*3;
        case Float4: return 4*4;
        case Mat3:   return 4*9;
        case Mat4:   return 4*16;
        }
        return 0;
    }

    inline static uint typeToNative(DataType type){
        switch (type)
        {
        case Int:    return GL_INT;
        case Float:
        case Float2:
        case Float3:
        case Float4:
        case Mat3:
        case Mat4:   return GL_FLOAT;
        }
        return 0;
    }

private:
    uint32 m_id = 0;
    std::string name;
    std::vector<std::shared_ptr<ShaderFile>> files;

    void loadFiles(std::vector<std::filesystem::path> filePaths);
    void compile();
};

