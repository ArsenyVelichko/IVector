cmake_minimum_required(VERSION 3.0)
project(Interfaces)

set(CMAKE_CXX_STANDARD 11)

add_compile_definitions(BUILD_INTERFACES)

include_directories(include)

set(UTILS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/Utils")

macro(create_lib_subdirectory LIB_NAME)
	project(${LIB_NAME})

	file(GLOB SOURCE_FILES *.cpp *.h)

	add_library(${LIB_NAME} ${SOURCE_FILES})
	target_include_directories(${LIB_NAME} PRIVATE ${UTILS_DIRECTORY})

	set(EXE_DEPENDENCIES ${EXE_DEPENDENCIES} ${LIB_NAME} PARENT_SCOPE)
endmacro()

add_subdirectory(src/Vector)
add_subdirectory(src/Set)
add_subdirectory(src/Logger)
add_subdirectory(src/Compact)

message(STATUS "${EXE_DEPENDENCIES}")

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${EXE_DEPENDENCIES})


