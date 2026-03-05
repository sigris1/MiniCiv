add_executable(backend_tests
        CoreGameTests.cpp
)

target_link_libraries(backend_tests PRIVATE
        GTest::gtest_main
        game_lib
)


include(GoogleTest)
gtest_discover_tests(backend_tests)