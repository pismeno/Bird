# It is highly recommended to use explicit file lists instead of GLOB,
# but if using GLOB, let's make sure it doesn't grab .mm files on Windows at all.
file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.hpp" "*.h")

# Remove everything in the external/ and build/ directories from CORE_SOURCES
list(FILTER CORE_SOURCES EXCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/external/.*")
list(FILTER CORE_SOURCES EXCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-.*")

# On Windows/MinGW, explicitly strip out any Objective-C++ (.mm) files if GLOB accidentally found them
if (NOT APPLE)
    list(FILTER CORE_SOURCES EXCLUDE REGEX ".*\\.mm$")
endif()

# Filter out Apple/Metal-exclusive files globally
list(FILTER CORE_SOURCES EXCLUDE REGEX "metal_renderer\\.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX "mtl_implementation\\.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX "mtl_implementation\\.mm")
list(FILTER CORE_SOURCES EXCLUDE REGEX "GLFWBridge\\.mm")

if (APPLE)
    # Re-add Apple/Metal files with explicit absolute paths
    list(APPEND CORE_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/metal/metal_renderer.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/metal/mtl_implementation.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/metal/mtl_implementation.mm"
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/metal/CocoaBridge.mm"
    )
    # Define macro for conditional preprocessor guarding in C++
    add_compile_definitions(BIRD_PLATFORM_APPLE)

elseif (UNIX AND NOT APPLE)
    add_compile_definitions(BIRD_PLATFORM_LINUX)

elseif (WIN32)
    add_compile_definitions(BIRD_PLATFORM_WINDOWS)
endif()

# Force SPIRV-Cross to use assertions instead of 'throw' since exceptions are disabled
add_compile_definitions(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=1)

# Now create the library using the filtered/curated list
add_library(BirdCore STATIC ${CORE_SOURCES})

# Find Vulkan on Windows
if (WIN32)
    find_package(Vulkan REQUIRED)
endif()

# Public dependencies
target_link_libraries(BirdCore
        PUBLIC
        glfw
        glm::glm
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-msl
        glslang
        glslang-default-resource-limits
        SPIRV
)

# Link Vulkan to BirdCore if it was found
if (Vulkan_FOUND)
    target_link_libraries(BirdCore PUBLIC Vulkan::Vulkan)
endif()

# Platform-specific private dependencies
if (APPLE)
    target_link_libraries(BirdCore PRIVATE
            "-framework Metal"
            "-framework Foundation"
            "-framework QuartzCore"
            "-framework Cocoa"
    )
endif()

# Ensure we include the directories correctly.
target_include_directories(BirdCore PUBLIC
        "${CMAKE_SOURCE_DIR}/external/metal-cpp"
        "${CMAKE_SOURCE_DIR}/external/glslang"
        "${CMAKE_SOURCE_DIR}/external/spirv-cross"
        "${CMAKE_SOURCE_DIR}/external"
        "${CMAKE_SOURCE_DIR}/external/stb"
        "${CMAKE_SOURCE_DIR}/external/json/include"
        "${CMAKE_SOURCE_DIR}/external/Vulkan-Headers/include"
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Automatically copy JSON definitions to the build directory every time BirdCore builds
add_custom_command(TARGET BirdCore POST_BUILD
        # Ensure the destination directory exists
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/node_system"
        # Copy the files if they have been modified
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/node_system/node_definitions.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/node_system/scene_definitions.json"
        "${CMAKE_BINARY_DIR}/node_system/"
        COMMENT "Copying node and scene JSON definitions to build directory"
)

add_custom_command(TARGET BirdCore POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders"
        "${CMAKE_BINARY_DIR}/shaders"
        COMMENT "Copying shaders to build directory"
)