cmake_minimum_required(VERSION 3.5)

project(IPZ_Client LANGUAGES CXX)
include_directories(src external)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(IPZ_Client
    external/olcPixelGameEngine.h
    external/olcPixelGameEngine.cpp
    src/graphics.h
    src/graphics.cpp
    src/main.cpp
    )
