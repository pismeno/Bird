add_executable(BirdEditor "main.cpp")

target_link_libraries(BirdEditor
    PRIVATE
    BirdCore
    "-framework Cocoa"
    "-framework QuartzCore"
    "-framework Metal"
    "-framework MetalKit"
    "-framework CoreGraphics"
    "glfw"
)

target_include_directories(BirdEditor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/external/metal-cpp"
)