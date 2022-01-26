cmake_minimum_required(VERSION 3.5)

project(Primor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS  OFF CACHE BOOL "" FORCE)


set(client_INCLUDE_DIRS
    external/stb
    external/glad/include
    external/glfw/include/GLFW
    external/glm/glm
    external/ppk_assert/src
    external/colorwin/src
    external/SIplusplus_wchar
    external/thread_pool
    external/assimp/include
    external/yaml-cpp/include
    external/imgui
    external/entt
    external/openal-soft/include
    external/dr_libs
    )
include_directories(${client_INCLUDE_DIRS})
link_directories(external/stb)
link_directories(external/entt)

file(GLOB_RECURSE client_SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

file(GLOB_RECURSE assert_SRC CONFIGURE_DEPENDS "external/ppk_assert/src/*.h" "external/ppk_assert/src/*.cpp")
file(GLOB_RECURSE SIplusplus_SRC CONFIGURE_DEPENDS "external/SIplusplus_wchar/*.h" "external/SIplusplus_wchar/*.cpp")
file(GLOB imgui_SRC CONFIGURE_DEPENDS
    "external/imgui/backends/imgui_impl_glfw.h"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_opengl3.h"
    "external/imgui/backends/imgui_impl_opengl3.cpp"
    "external/imgui/*.cpp"
    "external/imgui/*.h"
    )

set(client_INCLUDE_SRCS
    ${assert_SRC}
    ${SIplusplus_SRC}
    ${imgui_SRC}
    )

add_subdirectory(external/glfw)
add_library(glad "external/glad/src/glad.c")

add_executable(Primor
    ${client_SRC}
    ${client_INCLUDE_SRCS}
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/external/assimp/lib/Release/assimp-vc142-mt.lib" AND
   EXISTS "${CMAKE_SOURCE_DIR}/external/assimp/contrib/zlib/Release/zlibstatic.lib")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(Primor
            ${CMAKE_SOURCE_DIR}/external/assimp/lib/Debug/assimp-vc142-mtd.lib
            ${CMAKE_SOURCE_DIR}/external/assimp/contrib/zlib/Debug/zlibstaticd.lib
            )
    else()
        target_link_libraries(Primor
            ${CMAKE_SOURCE_DIR}/external/assimp/lib/Release/assimp-vc142-mt.lib
            ${CMAKE_SOURCE_DIR}/external/assimp/contrib/zlib/Release/zlibstatic.lib
            )
    endif()

else()
   add_subdirectory(external/assimp)
   target_link_libraries(Primor assimp)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/external/openal-soft/bin/Release/OpenAL32.lib")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(Primor
            ${CMAKE_SOURCE_DIR}/external/openal-soft/bin/Debug/OpenAL32.lib
            winmm
            )
    else()
        target_link_libraries(Primor
            ${CMAKE_SOURCE_DIR}/external/openal-soft/bin/Release/OpenAL32.lib
            winmm
            )
    endif()

else()
   add_subdirectory(external/openal-soft)
   target_link_libraries(Primor OpenAL32 winmm)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/external/yaml-cpp/bin/Release/yaml-cpp.lib")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(Primor
            ${CMAKE_SOURCE_DIR}/external/yaml-cpp/bin/Debug/yaml-cppd.lib
            )
    else()
        target_link_libraries(Primor
            ${CMAKE_SOURCE_DIR}/external/yaml-cpp/bin/Release/yaml-cpp.lib
            )
    endif()
else()
   add_subdirectory(external/yaml-cpp)
   target_link_libraries(Primor yaml-cpp)
endif()


target_link_libraries(Primor glfw glad opengl32)



