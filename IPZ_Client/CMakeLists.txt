cmake_minimum_required(VERSION 3.5)

project(IPZ_Client LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


add_executable(
    IPZ_Client
    src/main.cpp
    src/asset_manager.h
    src/assets.h
    )

add_subdirectory(external/glfw)
target_link_libraries(IPZ_Client glfw glad opengl32)
add_library(glad "external/glad/src/gl.c")

include_directories(
    src
    external/stb
    external/glad
    external/glfw/include/GLFW
    external/glm
    )

