#pragma once
#include "colorwin.hpp"
#define PPK_ASSERT_ENABLED 1
#define PPK_ASSERT_DISABLE_STL
#define PPK_ASSERT_DEFAULT_LEVEL Error
#include "ppk_assert.h"
#include <cstdint>
#include <iostream>
#include <glad/glad.h>
#include "math.h"

#define LOGGING_ENABLED


#define ASSERT                PPK_ASSERT
#define ASSERT_WARNING        PPK_ASSERT_WARNING
#define ASSERT_DEBUG          PPK_ASSERT_DEBUG
#define ASSERT_ERROR          PPK_ASSERT_ERROR
#define ASSERT_FATAL          PPK_ASSERT_FATAL
#define LOG(message, ...)          PPK_ASSERT_CUSTOM(0, 0, message, __VA_ARGS__)
#define WARN(message, ...)         PPK_ASSERT_CUSTOM(1, 0, message, __VA_ARGS__)
#define OPENGL_LOG(message, ...)   PPK_ASSERT_CUSTOM(2, 0, message, __VA_ARGS__)
#define OPENGL_THROW(message, ...) PPK_ASSERT_CUSTOM(3, 0, message, __VA_ARGS__)
#define GLFW_LOG(message, ...)     PPK_ASSERT_CUSTOM(4, 0, message, __VA_ARGS__)
#define GLFW_THROW(message, ...)   PPK_ASSERT_CUSTOM(5, 0, message, __VA_ARGS__)
#define ASSERT_NOT_REACHED()       PPK_ASSERT_CUSTOM(6, 0)

static ppk::assert::implementation::AssertAction::AssertAction assertHandler(const char* file, int line, const char* function,
                                                                              const char* expression, int level, const char* message)
{
    //TODO: make it nice
    using namespace ppk::assert::implementation;
    using namespace colorwin;
    switch(level)
    {
    case 0/*LOG */: std::cout<<color(cyan)<<message; return AssertAction::None;
    case 1/*WARN*/: std::cout<<color(red)<<file<<" : "<<line<<"\n"<<message<<"\n\n"; return AssertAction::None;
    case 2/*OPENGL_LOG  */: std::cout<<color(cyan)<<"[OpenGL] "<<message<<"\n\n"; return AssertAction::None;
    case 3/*OPENGL_THROW*/: std::cout<<color(red)<<"[OpenGL] "<<message<<"\n\n"; return AssertAction::Break;
    case 4/*GLFW_LOG    */: std::cout<<color(cyan)<<"[GLFW] "<<message<<"\n\n"; return AssertAction::None;
    case 5/*GLFW_THROW  */: std::cout<<color(red)<<"[GLFW] "<<message<<"\n\n"; return AssertAction::Break;
    case 6/*NOT_REACHED */: std::cout<<color(red)<<file<<" : "<<line<<"\n ASSERT_NOT_REACHED: Shouldn't be here!\n"; return AssertAction::Break;

    case AssertLevel::Warning: std::cout<<file<<line<<message; return AssertAction::None;
    case AssertLevel::Debug:
    case AssertLevel::Error:
    case AssertLevel::Fatal:
        std::cout<<file<<line<<message;
        return AssertAction::Break;
    }
    std::cout<<file<<" "<<line<<"Invalid Assert level:"<<level<<"\n\n"<<message;
    return AssertAction::Break;
}

#define UNUSED(x) (void)(x)

inline GLenum textureSizedFormatToFormat(GLenum internalFormat)
{
    switch(internalFormat)
    {
    case GL_R8:             return GL_RED;
    case GL_R8_SNORM:       return GL_RED;
    case GL_R16:            return GL_RED;
    case GL_R16_SNORM:      return GL_RED;
    case GL_RG8:            return GL_RG;
    case GL_RG8_SNORM:      return GL_RG;
    case GL_RG16:           return GL_RG;
    case GL_RG16_SNORM:     return GL_RG;
    case GL_R3_G3_B2:       return GL_RGB;
    case GL_RGB4:           return GL_RGB;
    case GL_RGB5:           return GL_RGB;
    case GL_RGB8:           return GL_RGB;
    case GL_RGB8_SNORM:     return GL_RGB;
    case GL_RGB10:          return GL_RGB;
    case GL_RGB12:          return GL_RGB;
    case GL_RGB16_SNORM:    return GL_RGB;
    case GL_RGBA2:          return GL_RGB;
    case GL_RGBA4:          return GL_RGB;
    case GL_RGB5_A1:        return GL_RGBA;
    case GL_RGBA8:          return GL_RGBA;
    case GL_RGBA8_SNORM:    return GL_RGBA;
    case GL_RGB10_A2:       return GL_RGBA;
    case GL_RGB10_A2UI:     return GL_RGBA;
    case GL_RGBA12:         return GL_RGBA;
    case GL_RGBA16:         return GL_RGBA;
    case GL_SRGB8:          return GL_RGB;
    case GL_SRGB8_ALPHA8:   return GL_RGBA;
    case GL_R16F:           return GL_RED;
    case GL_RG16F:          return GL_RG;
    case GL_RGB16F:         return GL_RGB;
    case GL_RGBA16F:        return GL_RGBA;
    case GL_R32F:           return GL_RED;
    case GL_RG32F:          return GL_RG;
    case GL_RGB32F:         return GL_RGB;
    case GL_RGBA32F:        return GL_RGBA;
    case GL_R11F_G11F_B10F: return GL_RGB;
    case GL_RGB9_E5:        return GL_RGB;
    case GL_R8I:            return GL_RED;
    case GL_R8UI:           return GL_RED;
    case GL_R16I:           return GL_RED;
    case GL_R16UI:          return GL_RED;
    case GL_R32I:           return GL_RED;
    case GL_R32UI:          return GL_RED;
    case GL_RG8I:           return GL_RG;
    case GL_RG8UI:          return GL_RG;
    case GL_RG16I:          return GL_RG;
    case GL_RG16UI:         return GL_RG;
    case GL_RG32I:          return GL_RG;
    case GL_RG32UI:         return GL_RG;
    case GL_RGB8I:          return GL_RGB;
    case GL_RGB8UI:         return GL_RGB;
    case GL_RGB16I:         return GL_RGB;
    case GL_RGB16UI:        return GL_RGB;
    case GL_RGB32I:         return GL_RGB;
    case GL_RGB32UI:        return GL_RGB;
    case GL_RGBA8I:         return GL_RGBA;
    case GL_RGBA8UI:        return GL_RGBA;
    case GL_RGBA16I:        return GL_RGBA;
    case GL_RGBA16UI:       return GL_RGBA;
    case GL_RGBA32I:        return GL_RGBA;
    case GL_RGBA32UI:       return GL_RGBA;
    case GL_DEPTH24_STENCIL8:   return GL_DEPTH_STENCIL;
    case GL_DEPTH_COMPONENT32F: return GL_DEPTH_COMPONENT;
    default: ASSERT_NOT_REACHED();
        return 0;
    }
}
