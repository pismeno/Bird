# Enables CTest
enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(BirdTests ${TEST_SOURCES})

target_link_libraries(BirdTests
        PRIVATE
        BirdCore
        doctest::doctest
)

add_test(NAME BirdTestSuite COMMAND BirdTests)