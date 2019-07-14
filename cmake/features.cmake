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

include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(FindPkgConfig)

check_function_exists(personality HAVE_PERSONALITY)
check_function_exists(unshare HAVE_UNSHARE)
check_function_exists(setgroups HAVE_SETGROUPS)

#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
#find_package(Threads REQUIRED)

set(cmake_position_independent_code true)
set(PRODUCEIT_LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}/produceit")

set(CMAKE_CXX_STANDARD 14)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_BUILD true)
    add_definitions(-DDEBUG)
endif()
