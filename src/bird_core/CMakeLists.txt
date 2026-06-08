# It is highly recommended to use explicit file lists instead of GLOB,
# but if using GLOB, let's make sure it doesn't grab .mm files on Windows at all.
file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.hpp" "*.h")

set(CORE_SOURCES
        "rendering/renderer.hpp"
        "rendering/renderer.cpp"
        "rendering/metal_renderer.cpp"
        "rendering/mtl_implementation.cpp"
        "rendering/stbi_image.cpp"
        "rendering/texture.hpp"
        "rendering/texture.cpp"
        "rendering/GLFWBridge.hpp"
        "rendering/GLFWBridge.mm"
        "rendering/shader_compiler.hpp"
        "rendering/shader_compiler.cpp"
        "utils/result.hpp"
        "windowing/iwindow.hpp"
        "windowing/iwindow.cpp"
        "windowing/glfw_window.hpp"
        "windowing/glfw_window.cpp"
        "inputs/inputs.hpp"
        "inputs/input_codes.hpp"
        "inputs/glfw_inputs.hpp"
        "inputs/glfw_inputs.cpp"
        "node_system/node_types.hpp"
        "node_system/node.hpp"
        "node_system/scene.hpp"
        "node_system/scene.cpp"
        "node_system/inode_manager.hpp"
        "node_system/inode_manager.cpp"
        "node_system/managers/transform_manager.hpp"
        "node_system/managers/transform_manager.cpp"
        "node_system/managers/drawable_manager.hpp"
        "node_system/managers/drawable_manager.cpp"
        "node_system/render_gatherer.cpp"
        "node_system/render_gatherer.hpp"
        "rendering/render_command.hpp"
)
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
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/metal_renderer.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/mtl_implementation.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/mtl_implementation.mm"
            "${CMAKE_CURRENT_SOURCE_DIR}/rendering/CocoaBridge.mm"
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
# NOTE: If your stb header is at `external/stb/stb_image.h`, use "${CMAKE_SOURCE_DIR}/external"
# If your stb header is at `external/stb/stb/stb_image.h`, use "${CMAKE_SOURCE_DIR}/external/stb"
target_include_directories(BirdCore PUBLIC
        "${CMAKE_SOURCE_DIR}/external/metal-cpp"
        "${CMAKE_SOURCE_DIR}/external/glslang"
        "${CMAKE_SOURCE_DIR}/external/spirv-cross"
        "${CMAKE_SOURCE_DIR}/external"      # Changed this to cover <stb/stb_image.h>
        "${CMAKE_SOURCE_DIR}/external/stb"  # Fallback
        ${CMAKE_CURRENT_SOURCE_DIR}
)