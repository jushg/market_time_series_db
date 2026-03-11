cmake_minimum_required(VERSION 3.14)

project(data_engine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE LIB_SOURCES
    "src/commands/*.cpp" "src/client/*.cpp"
    "src/models/*.cpp" "src/shared/*.cpp" "src/storage/*.cpp"
)
add_library(data_engine_lib STATIC ${LIB_SOURCES})
target_include_directories(data_engine_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(data_engine_lib PUBLIC cxx_std_17)
target_compile_options(data_engine_lib PUBLIC -Wall -ggdb)

add_executable(data_engine src/engine.cpp)
target_link_libraries(data_engine PRIVATE data_engine_lib)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
