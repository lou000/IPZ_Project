#pragma once
#include "colorwin.hpp"
#define PPK_ASSERT_ENABLED 1
#define PPK_ASSERT_DISABLE_STL
#define PPK_ASSERT_DEFAULT_LEVEL Error
#include "ppk_assert.h"
#include <cstdint>
#include <iostream>
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#define LOGGING_ENABLED


#define ASSERT                PPK_ASSERT
#define ASSERT_WARNING        PPK_ASSERT_WARNING
#define ASSERT_DEBUG          PPK_ASSERT_DEBUG
#define ASSERT_ERROR          PPK_ASSERT_ERROR
#define ASSERT_FATAL          PPK_ASSERT_FATAL
#define LOG(message, ...)     PPK_ASSERT_CUSTOM(0, 0, message, __VA_ARGS__)
#define WARN(message, ...)     PPK_ASSERT_CUSTOM(1, 0, message, __VA_ARGS__)
#define OPENGL_LOG(message, ...)     PPK_ASSERT_CUSTOM(2, 0, message, __VA_ARGS__)
#define OPENGL_THROW(message, ...)   PPK_ASSERT_CUSTOM(3, 0, message, __VA_ARGS__)
#define GLFW_LOG(message, ...)     PPK_ASSERT_CUSTOM(4, 0, message, __VA_ARGS__)
#define GLFW_THROW(message, ...)   PPK_ASSERT_CUSTOM(5, 0, message, __VA_ARGS__)
//TODO: Add LOG_ONCE and WARN_ONCE macros!
// std::unordered_set<size_t> hashujesz i dodajesz tutaj jak wyjdzie ze juz tam bylo to nie wyswietlasz loga a jak udalo sie dodac to wyswietlasz

static ppk::assert::implementation::AssertAction::AssertAction assertHandler(const char* file, int line, const char* function,
                                                                              const char* expression, int level, const char* message)
{
    //TODO: DAAAAAAWID zrób to ładne i zrób do tego timestamp
    using namespace ppk::assert::implementation;
    using namespace colorwin;
    switch(level)
    {
    // (13:12:45.245) [OpenGL] messege
    //W funkcji log nie potrzebujemy expression ani fuction
    case 0/*LOG */: std::cout<<file<<" : "<<line<<"\n"<<message<<"\n\n"; return AssertAction::None;
    //Tutaj powinniśmy wypisać wszystko co mamy
    case 1/*WARN*/: std::cout<<color(red)<<file<<" : "<<line<<"\n"<<message<<"\n\n"; return AssertAction::None;

    case 2/*OPENGL_LOG  */: std::cout<<color(cyan)<<"[OpenGL] "<<message<<"\n\n"; return AssertAction::None;
    case 3/*OPENGL_THROW*/: std::cout<<color(red)<<"[OpenGL] "<<message<<"\n\n"; return AssertAction::Throw;
    case 4/*GLFW_LOG    */: std::cout<<color(cyan)<<"[GLFW] "<<message<<"\n\n"; return AssertAction::None;
    case 5/*GLFW_THROW  */: std::cout<<color(red)<<"[GLFW] "<<message<<"\n\n"; return AssertAction::Throw;

    case AssertLevel::Warning: std::cout<<file<<line<<message; return AssertAction::None;
    case AssertLevel::Debug:
    case AssertLevel::Error:
    case AssertLevel::Fatal:
        std::cout<<file<<line<<message;
        return AssertAction::Throw;
    }
    std::cout<<file<<" "<<line<<"Invalid Assert level:"<<level<<"\n\n"<<message;
    return AssertAction::Throw;
}

#define UNUSED(x) (void)(x)


// becouse i hate std random generators
// this code is borrowed from OneLoneCoder
// https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_ProcGen_Universe.cpp
// who in turn borrowed rnd function from https://lemire.me/blog/2019/03/19/the-fastest-conventional-random-number-generator-that-can-pass-big-crush/

static uint32_t ___seed = 1;

inline uint32_t rnd()
{
    ___seed += 0xe120fc15;
    uint64_t tmp;
    tmp = (uint64_t)___seed * 0x4a39b70d;
    uint32_t m1 = (uint32_t)((tmp >> 32) ^ tmp);
    tmp = (uint64_t)m1 * 0x12fad5c9;
    uint32_t m2 = (uint32_t)((tmp >> 32) ^ tmp);
    return m2;
}

inline double rndDouble(double min, double max)
{
    return ((double)rnd() / (double)(0x7FFFFFFF)) * (max - min) + min;
}

inline int rndInt(int min, int max)
{
    if(min-max == 0) return min;
    return (rnd() % (max - min)) + min;
}

inline void rndSeed(uint32_t s)
{
    ___seed = s;
}
