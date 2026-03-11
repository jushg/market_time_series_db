file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(data_engine_tests ${TEST_SOURCES})
target_link_libraries(data_engine_tests PRIVATE data_engine_lib GTest::gtest_main)
target_include_directories(data_engine_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
)
include(GoogleTest)
gtest_discover_tests(data_engine_tests)
