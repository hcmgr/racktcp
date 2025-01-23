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

file (GLOB MAIN_SOURCES
    "${SRC_DIR}/main.cpp"
)

file (GLOB LISTENER_SOURCES 
    "${SRC_DIR}/listener.cpp"
)

add_executable(thread ${THREAD_SOURCES})
add_executable(main ${MAIN_SOURCES})
add_executable(listener ${LISTENER_SOURCES})