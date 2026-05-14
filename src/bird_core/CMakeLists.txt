file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.hpp" "*.h" "*.mm")

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
        "windowing/window.hpp"
        "windowing/window.cpp"
        "windowing/glfw_window.hpp"
        "windowing/glfw_window.cpp"
        "inputs/inputs.hpp"
        "inputs/input_codes.hpp"
        "inputs/glfw_inputs.hpp"
        "inputs/glfw_inputs.cpp"
        "node_system/node_types.hpp"
        "node_system/node.hpp"
        "node_system/node.cpp"
        "node_system/nodes/node_2d.hpp"
        "node_system/nodes/node_2d.cpp"
        "node_system/scene.hpp"
        "node_system/scene.cpp"
        "node_system/managers/transform/transforms.hpp"
        "node_system/managers/transform/transform_manager.hpp"
        "node_system/managers/transform/transform_manager.cpp"
)

add_library(BirdCore STATIC ${CORE_SOURCES})


target_link_libraries(BirdCore
        PUBLIC
        glfw
        spirv-cross-core
        glslang
    glslang-default-resource-limits
        SPIRV
        "-framework Metal"
        "-framework Foundation"
)

target_include_directories(BirdCore PUBLIC 
    "${CMAKE_SOURCE_DIR}/external/metal-cpp"
    "${CMAKE_SOURCE_DIR}/external/glslang"
)

find_library(METAL_FRAMEWORK Metal)
find_library(QUARTZ_FRAMEWORK QuartzCore)
find_library(FOUNDATION_FRAMEWORK Foundation)

target_link_libraries(BirdCore PUBLIC 
    ${METAL_FRAMEWORK} 
    ${QUARTZ_FRAMEWORK} 
    ${FOUNDATION_FRAMEWORK}
)
target_link_libraries(BirdCore PUBLIC glfw glm::glm)

target_include_directories(BirdCore
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC "${CMAKE_SOURCE_DIR}/external"
    PUBLIC "${CMAKE_SOURCE_DIR}/external/spirv-cross"
)

target_link_libraries(BirdCore
    PRIVATE
    spirv-cross-glsl
    spirv-cross-msl
)
