project(racktcp)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR "${CMAKE_SOURCE_DIR}")

file(GLOB SOURCES
    "${SRC_DIR}/*.cpp"
)
add_executable(main ${SOURCES})

