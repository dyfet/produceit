include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

add_executable(test_output  test_output.cpp)
add_dependencies(test_output common)
target_link_libraries(test_output common)
add_test(NAME test-output COMMAND test_output)
