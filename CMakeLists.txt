# Copyright (C) 2017-2019 David Sugar <tychosoft@gmail.com>.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

cmake_minimum_required(VERSION 3.1.0)
project(ProduceIt VERSION 0.4.1 LANGUAGES CXX)
set(COPYRIGHT 2019)
set(CMAKE_CXX_STANDARD 14)

include(cmake/custom.cmake OPTIONAL)
include(cmake/project.cmake)
include(cmake/features.cmake)
include(cmake/coverage.cmake)
include(cmake/deploy.cmake OPTIONAL)

# Setup headers

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.hpp.in config.hpp)
configure_file(produceit.spec.in produceit.spec)

# Files, common targets

file(GLOB scripts utils/*.rb)
file(GLOB bman1 src/*.1)
file(GLOB uman1 utils/*.1)
file(GLOB markdown *.md)

add_subdirectory(src)

# These give access to docs and support files in an ide

add_custom_target(manpages SOURCES ${bman1} ${uman1})
add_custom_target(scripts SOURCES ${scripts})
add_custom_target(support SOURCES produceit.conf config.hpp.in Doxyfile .rubocop.yml produceit.spec.in)

# Install targets

install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/deb-release.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME deb-release )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/deb-archive.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME deb-archive )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/deb-remove.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME deb-remove )
install(FILES ${CMAKE_SOURCE_DIR}/produceit.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
install(FILES ${CMAKE_SOURCE_DIR}/permissions DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/permissions.d RENAME produceit)

install(FILES ${uman1} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# testing

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    enable_testing()
    add_subdirectory(tests)
endif()
