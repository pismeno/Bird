add_executable(BirdEditor "main.cpp")

target_link_libraries(BirdEditor PRIVATE BirdCore)

target_include_directories(BirdEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})