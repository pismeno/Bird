set(EDITOR_SOURCES
        "main.cpp"
        "window.hpp"
        "window.cpp"
        "glfw_window.hpp"
        "glfw_window.cpp"
)

add_executable(BirdEditor ${EDITOR_SOURCES})

target_link_libraries(BirdEditor PRIVATE BirdCore)

target_include_directories(BirdEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})