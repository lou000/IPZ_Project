#include "shader.h"

Shader::Shader(const std::string &name, std::vector<std::filesystem::path> filePaths)
    :name(name)
{
    loadFiles(filePaths);
    compile();
}

void Shader::loadFiles(std::vector<std::filesystem::path> filePaths)
{
    for(auto& path : filePaths)
    {
        auto shaderFile = std::make_shared<ShaderFile>(path);
        if(shaderFile->data != nullptr)
        {
            files.push_back(shaderFile);
            AssetManager::addAsset(shaderFile);
        }
        else
            ASSERT_WARNING(0, "Failed to load shader");
    }
}

void Shader::compile()
{

}
