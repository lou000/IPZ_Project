#include "shader.h"
#include <string.h>
#include "../AssetManagement/asset_manager.h"

Shader::Shader(const std::string &name, std::vector<ShaderFileDef> files)
    :name(name), m_files(files)
{
    loadFiles();
    compile();
}

Shader::~Shader()
{
    if(m_id != 0)
        glDeleteProgram(m_id);

}

void Shader::bind()
{
    ASSERT_ERROR(m_id, "Shader: Couldnt bind shader, the id is invalid");
    glUseProgram(m_id);
}

void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::dispatch(uint x, uint y, uint z)
{
    ASSERT_ERROR(m_id, "Shader: Couldnt dispatch compute shader, the id is invalid");
    ASSERT_ERROR(isCompute, "Shader: Cant dispatch a non-compute shader dude!");
    glUseProgram(m_id);
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Shader::bindImage(std::shared_ptr<Texture> texture, uint unit, GLenum access, bool layered, uint layer)
{
    glBindImageTexture(unit, texture->id(), 0, layered, layer, access, texture->glFormatSized());
}


void Shader::setUniform(const char* name, BufferElement::DataType type, const std::any& value, bool transpose)
{
    int loc = glGetUniformLocation(m_id, name);
    switch (type)
    {
    case BufferElement::Uint:   glUniform1ui(loc, std::any_cast<uint>(value)); return;
    case BufferElement::Int:   glUniform1i(loc, std::any_cast<int>(value)); return;
    case BufferElement::Float: glUniform1f(loc, std::any_cast<float>(value)); return;
    case BufferElement::Float2:
    {
        auto val = std::any_cast<vec2>(value);
        glUniform2f(loc, val.x, val.y);
        return;
    }
    case BufferElement::Float3:
    {
        auto val = std::any_cast<vec3>(value);
        glUniform3f(loc, val.x, val.y, val.z);
        return;
    }
    case BufferElement::Float4:
    {
        auto val = std::any_cast<vec4>(value);
        glUniform4f(loc, val.x, val.y, val.z, val.w);
        return;
    }
    case BufferElement::Mat3: glUniformMatrix3fv(loc, 1, transpose, value_ptr(std::any_cast<mat3>(value))); return;
    case BufferElement::Mat4: glUniformMatrix4fv(loc, 1, transpose, value_ptr(std::any_cast<mat4>(value))); return;
    }
}

void Shader::setUniformArray(const char *name, BufferElement::DataType type, const std::any& values, uint count, bool transpose)
{
    int loc = glGetUniformLocation(m_id, name);
    switch (type)
    {
    case BufferElement::Uint:   glUniform1uiv(loc, count, std::any_cast<uint*>(values)); return;
    case BufferElement::Int:    glUniform1iv(loc, count, std::any_cast<int*>(values)); return;
    case BufferElement::Float:  glUniform1fv(loc, count, std::any_cast<float*>(values)); return;
        //everything below is untested, i have no clue if this works
    case BufferElement::Float2: glUniform2fv(loc, count*2, std::any_cast<float*>(values)); return;
    case BufferElement::Float3: glUniform3fv(loc, count*3, std::any_cast<float*>(values)); return;
    case BufferElement::Float4: glUniform4fv(loc, count*4, std::any_cast<float*>(values)); return;
    case BufferElement::Mat3:   glUniformMatrix3fv(loc, count*9, transpose, std::any_cast<float*>(values)); return;
    case BufferElement::Mat4:   glUniformMatrix4fv(loc, count*16, transpose, std::any_cast<float*>(values)); return;
    }
}

void Shader::updateRuntimeModifiedStrings(ShaderFileDef def)
{
    for(auto& fDef : m_files)
    {
        if(fDef.filepath == def.filepath)
        {
            fDef.stringsToReplace = def.stringsToReplace;
            fDef.replacementStrings = def.replacementStrings;
            LOG("Shader: Shader modified at runtime, recompiling...\n");
            compile();
            return;
        }
    }
}

void Shader::loadFiles()
{
    for(auto& shaderDef : m_files)
    {
        auto shaderFile = std::make_shared<ShaderFile>(shaderDef.filepath, name);
        if(shaderFile->text.length()>0)
        {
            shaderDef.file = shaderFile;
            AssetManager::addAsset(shaderFile);
        }
        else
            WARN("Shader: Failed to load shader file %s.", shaderDef.filepath.string().c_str());
    }
}

void Shader::compile()
{
    // We never delete the program even if we fail, in hope that
    // user will fix their shader and recompile the program.

    if(m_files.size() == 0)
    {
        WARN("Shader: There are no files to compile.");
        return;
    }

    GLuint program = m_id == 0 ? glCreateProgram() : m_id;
    m_id = program;
    std::vector<uint> shaders;

    bool greatSuccess = true;
    for(auto& fileDef : m_files)
    {
        auto sFile = fileDef.file;

        if(sFile->type == ShaderFile::ShaderType::compute)
            isCompute = true;

        GLuint shader = glCreateShader(sFile->type);

        auto text = sFile->text;
        ASSERT_ERROR(fileDef.stringsToReplace.size() == fileDef.replacementStrings.size(),
         "Shader: String replacement arrays are not equal size!");
        for(uint i=0; i<fileDef.stringsToReplace.size(); i++)
        {
            auto mark = fileDef.stringsToReplace[i];
            auto repl = fileDef.replacementStrings[i];
            size_t pos = text.find(mark);
            while( pos != std::string::npos)
            {
                text.replace(pos, mark.size(), repl);
                pos =text.find(mark, pos + mark.size());
            }
        }

        const char* cstr = text.c_str();

        glShaderSource(shader, 1, &cstr, 0);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if(success == 0)
        {
            GLint logSize = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

            std::vector<GLchar> errorLog(logSize);
            glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);
            glDeleteShader(shader);
            WARN("Shader: Shader %s compilation failed with error:\n%s", sFile->path.string().c_str(), errorLog.data());
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
            WARN("Shader: Program %s linking failed with error %s", name.c_str(), errorLog.data());
        }
        for(auto shader : shaders)
        {
            glDetachShader(program, shader);
            glDeleteShader(shader);
        }
    }
}


ShaderFileDef::ShaderFileDef(std::filesystem::path filepath, std::vector<std::string> strToReplace, std::vector<std::string> replStrings)
    :filepath(filepath), stringsToReplace(strToReplace), replacementStrings(replStrings)
{

}
