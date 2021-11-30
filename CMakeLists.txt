 cmake_minimum_required(VERSION 3.5)

project(Primor LANGUAGES C CXX)
add_compile_options("/FS")

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
    external/Eigen
    external/csv-parser
    external/assimp/include
    )
include_directories(${client_INCLUDE_DIRS})
link_directories(external/stb)

file(GLOB_RECURSE client_SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

file(GLOB_RECURSE assert_SRC CONFIGURE_DEPENDS "external/ppk_assert/src/*.h" "external/ppk_assert/src/*.cpp")
file(GLOB_RECURSE SIplusplus_SRC CONFIGURE_DEPENDS "external/SIplusplus_wchar/*.h" "external/SIplusplus_wchar/*.cpp")

set(client_INCLUDE_SRCS
    ${assert_SRC}
    ${SIplusplus_SRC}
    )

add_subdirectory(external/glfw)
add_subdirectory(external/assimp)
set_property(DIRECTORY external/assimp APPEND PROPERTY COMPILE_OPTIONS "/FS")
add_library(glad "external/glad/src/glad.c")

add_executable(Primor
    ${client_SRC}
    ${client_INCLUDE_SRCS}
    )
target_link_libraries(Primor glfw glad opengl32 assimp)



