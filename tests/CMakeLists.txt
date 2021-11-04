project(Tests)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

include_directories(TestUtils)

add_executable(run_tests ${SOURCE_FILES})
target_link_libraries(run_tests Logger Vector Set Compact ${CMAKE_DL_LIBS})