set(EDITOR_SOURCES
        "main.cpp"
)

add_executable(BirdEditor ${EDITOR_SOURCES})

target_link_libraries(BirdEditor PRIVATE BirdCore)

target_include_directories(BirdEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})