cmake_minimum_required(VERSION 2.8)
project(memhook)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)
