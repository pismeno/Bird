cmake_minimum_required(VERSION 3.28)

# set the project name and languages
project(Bird LANGUAGES CXX)

# set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# external library flags
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_MSL ON CACHE BOOL "" FORCE)
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)

# external subdirectories
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/spirv-cross)
add_subdirectory(external/glslang)
add_subdirectory(external/doctest)

# project subdirectories
add_subdirectory(src/bird_core)
add_subdirectory(src/bird_editor)
add_subdirectory(tests/)