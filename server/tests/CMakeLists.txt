add_executable(server_test
        test_main.cpp
        ServerTests.cpp
        HttpTestClient.h
)

target_link_libraries(server_test PRIVATE
        GTest::gtest
        server_lib
)

target_include_directories(server_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

enable_testing()
include(GoogleTest)
gtest_discover_tests(server_test)