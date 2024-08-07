cmake_minimum_required(VERSION 3.15)
project(neo)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(neo ${SOURCE_FILES})
