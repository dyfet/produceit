# Copyright (C) 2018-2019 David Sugar <tychosoft@gmail.com>.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

add_executable(test_output  test_output.cpp)
add_dependencies(test_output common)
target_link_libraries(test_output common)
add_test(NAME test-output COMMAND test_output)

add_executable(test_strings  test_strings.cpp)
add_dependencies(test_strings common)
target_link_libraries(test_strings common)
add_test(NAME test-strings COMMAND test_strings)

add_executable(test_keyfile  test_keyfile.cpp)
add_dependencies(test_keyfile common)
target_link_libraries(test_keyfile common)
add_test(NAME test-keyfile COMMAND test_keyfile)
