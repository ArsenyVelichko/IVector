project(Set)

file(GLOB SOURCE_FILES *.cpp *.h)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${UTILS_DIRECTORY})
target_link_libraries(${PROJECT_NAME} Logger Utils Vector)