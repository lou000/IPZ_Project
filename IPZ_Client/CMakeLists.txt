cmake_minimum_required(VERSION 3.5)

project(IPZ_Client LANGUAGES CXX)
include_directories(src external)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

target_link_libraries(glfw)

include_directories(src external/stb external/olc external/glfw/include/GLFW external/glm)

add_executable(IPZ_Client src/main.cpp src/olcPixelGameEngine.cpp)
