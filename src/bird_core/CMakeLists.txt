set(CORE_SOURCES
        "result.hpp"
        "window.hpp"
        "window.cpp"
        "glfw_window.hpp"
        "glfw_window.cpp"
        "inputs/inputs.hpp"
        "inputs/input_codes.hpp"
        "inputs/glfw_inputs.hpp"
        "inputs/glfw_inputs.cpp"
)

add_library(BirdCore STATIC ${CORE_SOURCES})

target_link_libraries(BirdCore PUBLIC glfw glm::glm)

target_include_directories(BirdCore
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)