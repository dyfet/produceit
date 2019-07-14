# Test for compiler and runtime library features.
# Copyright (C) 2019 David Sugar, Tycho Softworks.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if(COVERAGE_TYPE MATCHES "gcov" AND CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
    add_custom_target(coverage
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND rm -f *.gcov
        COMMAND tests/test_output
        COMMAND tests/test_strings
        COMMAND tests/test_keyfile
        COMMAND gcov -b src/*.cpp -o CMakeFiles/common.dir/src
    )
endif()
