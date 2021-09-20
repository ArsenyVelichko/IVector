cmake_minimum_required(VERSION 3.0)
project(IVector)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
file(GLOB SRC src/*.cpp src/*.h)

add_compile_definitions(BUILD_INTERFACES)
add_executable(${PROJECT_NAME} ${SRC})
