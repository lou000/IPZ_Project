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
    if(m_id != 0)
        glDeleteProgram(m_id);

}

void Shader::bind()
{
    if(m_id != 0)
        glUseProgram(m_id);
    else
        ASSERT_WARNING(0, "Shader: Couldnt bind shader, the id is invalid");
}

void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::setUniform(const char* name, DataType type, const std::any& value, bool transpose)
{
    int loc = glGetUniformLocation(m_id, name);
    switch (type)
    {
    case Int:   glUniform1i(loc, std::any_cast<int>(value)); return;
    case Float: glUniform1f(loc, std::any_cast<float>(value)); return;
    case Float2:
    {
        auto val = std::any_cast<vec2>(value);
        glUniform2f(loc, val.x, val.y);
        return;
    }
    case Float3:
    {
        auto val = std::any_cast<vec3>(value);
        glUniform3f(loc, val.x, val.y, val.z);
        return;
    }
    case Float4:
    {
        auto val = std::any_cast<vec4>(value);
        glUniform4f(loc, val.x, val.y, val.z, val.w);
        return;
    }
    case Mat3: glUniformMatrix3fv(loc, 1, transpose, value_ptr(std::any_cast<mat3>(value))); return;
    case Mat4: glUniformMatrix4fv(loc, 1, transpose, value_ptr(std::any_cast<mat4>(value))); return;
    }
}

void Shader::setUniformArray(const char *name, Shader::DataType type, const std::any& values, uint count, bool transpose)
{
    int loc = glGetUniformLocation(m_id, name);
    switch (type)
    {
    case Int:    glUniform1iv(loc, count, std::any_cast<int*>(values)); return;
    case Float:  glUniform1fv(loc, count, std::any_cast<float*>(values)); return;
        //everything below is untested, i have no clue if this works
    case Float2: glUniform2fv(loc, count, value_ptr(*std::any_cast<vec2*>(values))); return;
    case Float3: glUniform3fv(loc, count, value_ptr(*std::any_cast<vec3*>(values))); return;
    case Float4: glUniform4fv(loc, count, value_ptr(*std::any_cast<vec4*>(values))); return;
    case Mat3:   glUniformMatrix3fv(loc, count, transpose, value_ptr(*std::any_cast<mat3*>(values))); return;
    case Mat4:   glUniformMatrix4fv(loc, count, transpose, value_ptr(*std::any_cast<mat4*>(values))); return;
    }
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

    GLuint program = m_id == 0 ? glCreateProgram() : m_id;
    m_id = program;
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
        GLint success = 0;
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

