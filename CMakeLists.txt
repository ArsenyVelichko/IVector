cmake_minimum_required(VERSION 3.0)
project(Interfaces)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(include)

set(UTILS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/Utils")

add_subdirectory(src/Utils)

add_subdirectory(src/Logger)
add_subdirectory(src/Vector)
add_subdirectory(src/Set)
add_subdirectory(src/Compact)
add_subdirectory(src/Problem)
add_subdirectory(src/Solver)

add_subdirectory(tests)