project(Utils)

add_definitions(-DBUILD_INTERFACES)

file(GLOB SOURCE_FILES *.cpp *.h)
add_library(${PROJECT_NAME} ${SOURCE_FILES})