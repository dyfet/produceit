# Project settings and common targets.
# Copyright (C) 2019 David Sugar, Tycho Softworks.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

string(TOLOWER "${PROJECT_NAME}" PROJECT_ARCHIVE)
string(REGEX REPLACE "[.]" "," RC_VERSION ${PROJECT_VERSION})
string(TIMESTAMP CURRENT_YEAR "%Y")

if(NOT UNIX)
    error("Unsupported platform")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX true)
endif()

if(NOT COPYRIGHT)
    set(PROJECT_COPYRIGHT "${CURRENT_YEAR}")
elseif(NOT ${CURRENT_YEAR} STREQUAL "${COPYRIGHT}")
    set(PROJECT_COPYRIGHT "${COPYRIGHT}-${CURRENT_YEAR}")
else()
    set(PROJECT_COPYRIGHT "${CURRENT_YEAR}")
endif()

include(GNUInstallDirs)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_custom_target(dist
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.tar.gz"
    COMMAND git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_ARCHIVE}-${PROJECT_VERSION}/" "v${PROJECT_VERSION}" 2>/dev/null || git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_ARCHIVE}-${PROJECT_VERSION}/" HEAD
)

if(RELEASE AND NOT PROJECT_RELEASE)
    set(PROJECT_RELEASE "${RELEASE}")
elseif(NOT PROJECT_RELEASE)
    set(PROJECT_RELEASE "0")
endif()
set(RC_VERSION "${RC_VERSION},${PROJECT_RELEASE}")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(EXISTS "/usr/local/lib/")
        include_directories("/usr/local/include")
        link_directories("/usr/local/lib")
    endif()
    if(EXISTS "/usr/pkg/lib/")
        include_directories("/usr/pkg/include")
        link_directories("/usr/pkg/lib")
    endif()
endif()
