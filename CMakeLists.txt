cmake_minimum_required(VERSION 3.5)

project(data_engine)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
# setting skeleton source
file(GLOB TARGET_SRC "./src/*.cpp" )
include_directories("./include")
add_executable(${PROJECT_NAME} ${TARGET_SRC})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -ggdb)


