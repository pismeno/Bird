# Enables CTest
enable_testing()

add_executable(BirdTests
        test_main.cpp
        test_test.cpp
        transform_manager_test.cpp
        material_pool_test.cpp
        material_packer_test.cpp
)

target_link_libraries(BirdTests
        PRIVATE
        BirdCore
        doctest::doctest
)

add_test(NAME BirdTestSuite COMMAND BirdTests)