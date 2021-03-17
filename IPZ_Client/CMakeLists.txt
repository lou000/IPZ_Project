cmake_minimum_required(VERSION 3.5)

project(IPZ_Client LANGUAGES CXX)
include_directories(src external)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(IPZ_Client
    external/olcPixelGameEngine.h
    external/olcPixelGameEngine.cpp
    external/olcPGEX_Graphics3D.h
    src/game.h
    src/graphics.h
    src/asset_manager.cpp
    src/main.cpp
    )
