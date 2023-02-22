cmake_minimum_required(VERSION 3.22)
project(pico C CXX ASM)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

enable_testing()


include_directories(
    "../include"
)


add_executable(
    ${PROJECT_NAME}_tests
    testRingBuffer.cpp
    testMessage.cpp
)


target_link_libraries(
    ${PROJECT_NAME}_tests
    PRIVATE gtest gtest_main Threads::Threads
)


include(GoogleTest)
gtest_discover_tests(
    ${PROJECT_NAME}_tests
    )