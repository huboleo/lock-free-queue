cmake_minimum_required(VERSION 3.20)

project(lock-free-queue CXX)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(lock-free-queue ${SOURCE_FILES})

target_include_directories(lock-free-queue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

option(LFQUEUE_BUILD_BENCHMARKS "Build LFQueue benchmarks" ON)

if(LFQUEUE_BUILD_BENCHMARKS)
    include(FetchContent)

    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        google_benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(google_benchmark)

    add_executable(lfqueue_benchmark benchmarks/spsc_benchmark.cpp)
    target_include_directories(lfqueue_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(lfqueue_benchmark PRIVATE benchmark::benchmark_main)
endif()
