add_executable(BirdEditor)

target_sources(BirdEditor
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/editor/editor.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/editor/editor_context.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/editor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(BirdEditor
        PRIVATE
        BirdCore
        glfw
)

target_include_directories(BirdEditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(APPLE)
    target_link_libraries(BirdEditor
            PRIVATE
            "-framework Cocoa"
            "-framework QuartzCore"
            "-framework Metal"
            "-framework MetalKit"
            "-framework CoreGraphics"
    )

    target_include_directories(BirdEditor PRIVATE
            "${CMAKE_SOURCE_DIR}/external/metal-cpp"
    )
endif()