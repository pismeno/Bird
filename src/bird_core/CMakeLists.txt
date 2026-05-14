set(CORE_SOURCES
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
        "node_system/imanager.hpp"
        "node_system/imanager.cpp"
        "node_system/managers/transform/transforms.hpp"
        "node_system/managers/transform/transform_manager.hpp"
        "node_system/managers/transform/transform_manager.cpp"
)

add_library(BirdCore STATIC ${CORE_SOURCES})

target_link_libraries(BirdCore PUBLIC glfw glm::glm)

target_include_directories(BirdCore
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})