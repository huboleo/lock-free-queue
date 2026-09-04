cmake_minimum_required(VERSION 3.20)

project(lock-free-queue CXX)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(lock-free-queue ${SOURCE_FILES})

target_include_directories(lock-free-queue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
