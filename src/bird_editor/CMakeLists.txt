add_executable(BirdEditor)

target_sources(BirdEditor
        PRIVATE
            "main.cpp"
)

target_link_libraries(BirdEditor
        PRIVATE
        BirdCore
        glfw
)

target_include_directories(BirdEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

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