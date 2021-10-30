project(Logger)

file(GLOB SOURCE_FILES *.cpp *.h)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
