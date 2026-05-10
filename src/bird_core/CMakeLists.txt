file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.hpp" "*.h")

add_library(BirdCore STATIC ${CORE_SOURCES})

target_link_libraries(BirdCore
        PUBLIC
        glfw
)

target_include_directories(BirdCore
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        # Add internal paths here if needed
)

set(CORE_SOURCES
        "BirdEngine.cpp"
        "BirdEngine.hpp"
)