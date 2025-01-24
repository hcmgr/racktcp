cmake_minimum_required(VERSION 3.9)
project(racktcp)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR "${CMAKE_SOURCE_DIR}")

file(GLOB THREAD_SOURCES
    "${SRC_DIR}/tcp.cpp"
    "${SRC_DIR}/buffer.cpp"
    "${SRC_DIR}/test_utils.cpp"
    "${SRC_DIR}/utils.cpp"
    "${SRC_DIR}/ip.cpp"
)

add_executable(thread1 ${THREAD_SOURCES})
target_compile_definitions(thread1 PRIVATE THREAD1)
add_executable(thread2 ${THREAD_SOURCES})
target_compile_definitions(thread2 PRIVATE THREAD2)