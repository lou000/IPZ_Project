cmake_minimum_required(VERSION 3.5)

project(Primor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


set(client_INCLUDE_DIRS
    external/stb
    external/glad/include
    external/glfw/include/GLFW
    external/glm/glm
    external/ppk_assert/src
    external/colorwin/src
    external/SIplusplus_wchar
    external/fast_obj
    external/thread_pool
    external/Eigen
    external/csv-parser
    external/imgui
    )
include_directories(${client_INCLUDE_DIRS})
link_directories(external/stb)

set(OpenCV_DIR C:/Users/user/Downloads/opencv/build)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

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

add_subdirectory(external/fast_obj)
add_subdirectory(external/glfw)
add_library(glad "external/glad/src/glad.c")

add_executable(Primor
    ${client_SRC}
    ${client_INCLUDE_SRCS}
    )
target_link_libraries(Primor glfw glad opengl32 ${OpenCV_LIBS})



