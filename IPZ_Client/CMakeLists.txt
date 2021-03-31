cmake_minimum_required(VERSION 3.5)

project(IPZ_Client LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


set(si_INCLUDE_DIRS
    external/stb
    external/glad
    external/glfw/include/GLFW
    external/glm
    external/assert)
include_directories(${si_INCLUDE_DIRS})
link_directories(external/stb)

file(GLOB_RECURSE client_SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")
file(GLOB_RECURSE assert_SRC CONFIGURE_DEPENDS "external/assert/*.h" "external/assert/*.cpp")

set(si_INCLUDE_SRCS
    ${assert_SRC}
    )


add_subdirectory(external/glfw)
add_library(glad "external/glad/src/gl.c")

add_executable(IPZ_Client ${client_SRC} ${client_INCLUDE_SRCS})
target_link_libraries(IPZ_Client glfw glad opengl32)



