file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.hpp" "*.h" "*.mm")

set(CORE_SOURCES
        "rendering/renderer.hpp"
        "rendering/renderer.cpp"
        "rendering/metal_renderer.cpp"
        "rendering/mtl_implementation.cpp"
        "rendering/stbi_image.cpp"
        "rendering/GLFWBridge.hpp"
        "rendering/GLFWBridge.mm"
        "rendering/shader_compiler.hpp"
        "rendering/shader_compiler.cpp"
        "result.hpp"
        "window.hpp"
        "window.cpp"
        "glfw_window.hpp"
        "glfw_window.cpp"
        "inputs/inputs.hpp"
        "inputs/input_codes.hpp"
        "inputs/glfw_inputs.hpp"
        "inputs/glfw_inputs.cpp"
        "nodes/node_types.hpp"
        "nodes/node.hpp"
        "nodes/node.cpp"
        "nodes/node_manager.hpp"
        "nodes/node_manager.cpp"
        "rendering/texture.hpp"
        "rendering/texture.cpp"
        
)

add_library(BirdCore STATIC ${CORE_SOURCES})

target_compile_definitions(BirdCore PUBLIC
    BIRD_SHADER_VERT_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/triangle.vert.glsl"
    BIRD_SHADER_FRAG_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders/triangle.frag.glsl"
    BIRD_METAL_SHADER_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/rendering/triangle.metal"
    BIRD_TEXTURE_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/rendering/alpha_test.png"
)



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
