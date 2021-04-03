#include "shader.h"
#include "asset_manager.h"

Shader::Shader(const std::string &name, std::vector<std::filesystem::path> filePaths)
    :name(name)
{
    loadFiles(filePaths);
    compile();
}

Shader::~Shader()
{
    if(id != 0)
        glDeleteProgram(id);

}

void Shader::bind()
{
    if(id != 0)
        glUseProgram(id);
    else
        ASSERT_WARNING(0, "Shader: Couldnt bind shader, the id is invalid");
}

void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::loadFiles(std::vector<std::filesystem::path> filePaths)
{
    for(auto& path : filePaths)
    {
        auto shaderFile = std::make_shared<ShaderFile>(path, name);
        if(shaderFile->data != nullptr)
        {
            files.push_back(shaderFile);
            AssetManager::addAsset(shaderFile);
        }
        else
            ASSERT_WARNING(0, "Shader: Failed to load shader file %s.", path.string().c_str());
    }
}

void Shader::compile()
{
    // We never delete the program even if we fail, in hope that
    // user will fix their shader and recompile the program.

    if(files.size() == 0)
    {
        ASSERT_WARNING(0, "Shader: There are no files to compile.");
        return;
    }

    GLuint program = id == 0 ? glCreateProgram() : id;
    std::vector<uint> shaders;

    bool greatSuccess = true;
    for(auto& file : files)
    {
        GLuint shader = glCreateShader(file->type);
        glShaderSource(shader, 1, &file->data, 0);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if(success == 0)
        {
            GLint logSize = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            ASSERT(logSize>0); //should never happen

            std::vector<GLchar> errorLog(logSize);
            glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);
            glDeleteShader(shader);
            ASSERT_WARNING(0, "Shader: Shader %s compilation failed with error %s", file->path.string().c_str(), errorLog.data());
            greatSuccess = false;
        }
        else
            shaders.push_back(shader);
    }

    if(greatSuccess)
    {
        for(auto shader : shaders)
            glAttachShader(program, shader);
        glLinkProgram(program);
        GLint success = 1;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(success == 0)
        {
            GLint logSize = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
            std::vector<GLchar> errorLog(logSize);
            glGetProgramInfoLog(program, logSize, &logSize, &errorLog[0]);
            ASSERT_WARNING(0, "Shader: Program %s linking failed with error %s", name.c_str(), errorLog.data());
        }
        for(auto shader : shaders)
        {
            glDetachShader(program, shader);
            glDeleteShader(shader);
        }
    }
}

