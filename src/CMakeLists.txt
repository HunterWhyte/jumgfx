cmake_minimum_required(VERSION 3.7)
project(skin_engine C)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(skin_engine ${SOURCES})

set(OPTS -g -Wall -Wextra -Wpedantic -Wformat)

target_include_directories(skin_engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(skin_engine PRIVATE ${OPTS})
target_link_options(skin_engine PRIVATE ${OPTS})