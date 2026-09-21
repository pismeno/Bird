if (APPLE)
    add_compile_definitions(BIRD_PLATFORM_APPLE)
elseif (UNIX AND NOT APPLE)
    add_compile_definitions(BIRD_PLATFORM_LINUX)
elseif (WIN32)
    add_compile_definitions(BIRD_PLATFORM_WINDOWS)
endif()

add_compile_definitions(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=1)

add_library(BirdUtils INTERFACE)

target_sources(BirdUtils INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/include/utils/result.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/include/utils/string_hash.hpp
)

target_include_directories(BirdUtils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/utils/include)

add_library(BirdOS STATIC)

target_sources(BirdOS
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/os/windowing/glfw/glfw_window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/os/windowing/glfw/glfw_window.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/os/windowing/iwindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/os/inputs/glfw/glfw_inputs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/os/inputs/glfw/glfw_inputs.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/os/include/os/iwindow.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/os/include/os/inputs.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/os/include/os/input_codes.hpp
)

target_include_directories(BirdOS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/os/include)
target_include_directories(BirdOS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(BirdOS PRIVATE
        glfw
        glm::glm
        BirdUtils
)

add_library(BirdResources STATIC)

target_sources(BirdResources
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/placeholder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/resources/render_command.hpp
)

target_include_directories(BirdResources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/resources/include)
target_include_directories(BirdResources PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(BirdResources PUBLIC
        BirdOS
        BirdUtils
)

add_library(BirdNodeSystem STATIC)

target_sources(BirdNodeSystem
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/inode_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/node_handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/render_gatherer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/scene_factory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/scene_serde.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/managers/drawable_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/managers/transform_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/inode_manager.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/node.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/node_handle.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/render_gatherer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/node_types.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/scene.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/scene_factory.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/managers/drawable_manager.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include/node_system/managers/transform_manager.hpp
)

target_include_directories(BirdNodeSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/node_system/include)
target_include_directories(BirdNodeSystem PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_SOURCE_DIR}/external/json/include"
)

target_link_libraries(BirdNodeSystem PUBLIC BirdResources)

target_link_libraries(BirdNodeSystem PRIVATE
        glm::glm
)

add_custom_command(TARGET BirdNodeSystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/node_system"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/node_system/node_definitions.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/node_system/scene_definitions.json"
        "${CMAKE_BINARY_DIR}/node_system/"
)

add_library(BirdRendering STATIC)

target_sources(BirdRendering
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/irenderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/shader_compiler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/stbi_image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/vulkan/vulkan_renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/vulkan/vulkan_renderer.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/include/rendering/irenderer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/include/rendering/shader_compiler.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/include/rendering/texture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering/include/rendering/vertex.hpp
)

target_include_directories(BirdRendering PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rendering/include)
target_include_directories(BirdRendering PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_SOURCE_DIR}/external/stb"
        "${CMAKE_SOURCE_DIR}/external/Vulkan-Headers/include"
)

target_link_libraries(BirdRendering PUBLIC BirdResources)

target_link_libraries(BirdRendering PRIVATE
        glm::glm
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-msl
        glslang
        glslang-default-resource-limits
        SPIRV
)

add_custom_command(TARGET BirdRendering POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/rendering/shaders"
        "${CMAKE_BINARY_DIR}/shaders"
)

add_library(BirdCore INTERFACE)

target_link_libraries(BirdCore INTERFACE
        BirdNodeSystem
        BirdRendering
        BirdResources
        BirdOS
)

if (WIN32)
    find_package(Vulkan REQUIRED)
endif()

target_link_libraries(BirdCore
        INTERFACE
        glm::glm
)

if (Vulkan_FOUND)
    target_link_libraries(BirdCore INTERFACE Vulkan::Vulkan)
endif()

if (APPLE)
    target_link_libraries(BirdCore INTERFACE
            "-framework Metal"
            "-framework Foundation"
            "-framework QuartzCore"
            "-framework Cocoa"
    )
endif()

target_include_directories(BirdCore INTERFACE
        "${CMAKE_SOURCE_DIR}/external/metal-cpp"
        "${CMAKE_SOURCE_DIR}/external/glslang"
        "${CMAKE_SOURCE_DIR}/external/spirv-cross"
        "${CMAKE_SOURCE_DIR}/external"
        "${CMAKE_SOURCE_DIR}/external/stb"
        "${CMAKE_SOURCE_DIR}/external/json/include"
        "${CMAKE_SOURCE_DIR}/external/Vulkan-Headers/include"
        ${CMAKE_CURRENT_SOURCE_DIR}
)