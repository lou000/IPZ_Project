#pragma once
#include <gtc/type_ptr.hpp>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <glm.hpp>
#include <memory>
#include <any>
#include "buffer.h"


using namespace glm;
class ShaderFile;

struct ShaderFileDef{
    ShaderFileDef(std::filesystem::path filepath,
                  std::vector<std::string> strToReplace = {},
                  std::vector<std::string> replStrings = {});
    std::filesystem::path filepath;
    std::vector<std::string> stringsToReplace;
    std::vector<std::string> replacementStrings;
    std::shared_ptr<ShaderFile> file;
};

class Shader
{
    friend class ShaderFile;
    friend class AssetManager;

public:
    Shader(const std::string& name, std::vector<ShaderFileDef> filePaths);
    ~Shader();

    void bind();
    void unbind();
    void dispatch(uint x, uint y, uint z);
    uint32 id() {return m_id;}
    void bindImage(std::shared_ptr<Texture> image, uint unit, GLenum access, bool layered = false, uint layer = 0);
    void setUniform(const char* name, BufferElement::DataType type, const std::any& value, bool transpose = false);
    void setUniformArray(const char* name, BufferElement::DataType type, const std::any& value, uint count, bool transpose = false);
    void updateRuntimeModifiedStrings(ShaderFileDef def);

private:
    uint32 m_id = 0;
    bool isCompute = false;
    std::string name;
    std::vector<ShaderFileDef> m_files;

    void loadFiles();
    void compile();
};

