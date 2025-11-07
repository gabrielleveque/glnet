cmake_minimum_required(VERSION 3.16)
project(glnet CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_NAME glnet)

file(GLOB_RECURSE SRC src/*.cpp)

add_library(${LIB_NAME} STATIC ${SRC})

target_include_directories(${LIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
