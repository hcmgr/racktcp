cmake_minimum_required(VERSION 3.9)
project(racktcp)

find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR "${CMAKE_SOURCE_DIR}/oldSrc")

file(GLOB THREAD_SOURCES
    "${SRC_DIR}/*.cpp"
)

add_executable(thread1 ${THREAD_SOURCES})
target_compile_definitions(thread1 PRIVATE THREAD1)
target_link_libraries(thread1 PRIVATE OpenSSL::Crypto)
target_include_directories(thread1 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(thread2 ${THREAD_SOURCES})
target_compile_definitions(thread2 PRIVATE THREAD2)
target_link_libraries(thread2 PRIVATE OpenSSL::Crypto)
target_include_directories(thread2 PRIVATE ${OPENSSL_INCLUDE_DIR})