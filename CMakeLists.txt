cmake_minimum_required(VERSION 3.9)
project(racktcp)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR "${CMAKE_SOURCE_DIR}")

file(GLOB MAIN_SOURCES
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/utils.cpp"
)

file(GLOB LISTENER_SOURCES
    "${SRC_DIR}/listener.cpp"
)

file(GLOB BUFFER_SOURCES
    "${SRC_DIR}/buffer.cpp"
    "${SRC_DIR}/test_utils.cpp"
)

# add_executable(main ${MAIN_SOURCES})
# add_executable(listener ${LISTENER_SOURCES})
add_executable(buffer ${BUFFER_SOURCES})

