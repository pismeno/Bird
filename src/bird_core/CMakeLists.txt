file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.hpp" "*.h" "*.mm")

set(CORE_SOURCES
        "rendering/renderer.hpp"
        "rendering/renderer.cpp"
        "rendering/metal_renderer.cpp"
        "rendering/mtl_implementation.cpp"
        "rendering/GLFWBridge.hpp"
        "rendering/GLFWBridge.mm"
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
)

add_library(BirdCore STATIC ${CORE_SOURCES})

target_compile_definitions(BirdCore PUBLIC
    BIRD_METAL_SHADER_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/rendering/triangle.metal"
)



target_link_libraries(BirdCore
        PUBLIC
        glfw
        "-framework Metal"
        "-framework Foundation"
)

target_include_directories(BirdCore PUBLIC 
    "${CMAKE_SOURCE_DIR}/external/metal-cpp"
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
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
