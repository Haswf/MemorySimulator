cmake_minimum_required(VERSION 3.16)
project(scheduler C)

set(CMAKE_C_STANDARD 99)
set (CMAKE_CXX_FLAGS "-fexceptions")
set(CMAKE_BUILD_TYPE "Debug")
# Add executable target with source files listed in SOURCE_FILES variable
FILE(GLOB_RECURSE SOURCE_FILE src/*.c)
add_executable(scheduler ${SOURCE_FILE} src/constants.c src/constants.h)
target_link_libraries(scheduler m)
