cmake_minimum_required(VERSION 3.5)

project(data_engine)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

file(GLOB SOURCES
    "./include*.hpp"
    "./src/commands/*.cpp"
    "./src/client/*.cpp"
    "./src/models/*.cpp"
    "./src/shared/*.cpp"
    "./src/storage/*.cpp"
    "./src/*.cpp"
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -ggdb)


