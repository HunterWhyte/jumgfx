cmake_minimum_required(VERSION 3.7)
project(skin_engine)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(skin_engine ${SOURCES})

set(OPTS -g -Wall -Wextra -Wpedantic -Wformat)

set(CYAML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/libcyaml")

target_include_directories(skin_engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/ext/libcyaml/include")
target_compile_options(skin_engine PRIVATE ${OPTS})
target_link_options(skin_engine PRIVATE ${OPTS})

add_library(cyaml STATIC IMPORTED)
set_target_properties(cyaml PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ext/libcyaml/build/debug/libcyaml.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/libcyaml/include"
)

target_link_libraries(skin_engine cyaml)

add_subdirectory(example)

add_subdirectory(test)
