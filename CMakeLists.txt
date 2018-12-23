cmake_minimum_required(VERSION 3.1.0)
project(ProduceIt VERSION 0.2.1 LANGUAGES CXX)
string(TOLOWER "${PROJECT_NAME}" PROJECT_ARCHIVE)
set(PROJECT_RELEASE "1")

if (POLICY CMP0063)
  cmake_policy(SET CMP0063 OLD)
endif()

if(CMAKE_COVERAGE_TYPE MATCHES "gcov")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

if(CMAKE_GENERATOR MATCHES "Unix")
    set(CMAKE_MAINTAINER true)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX true)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/Custom.cmake OPTIONAL)
include(GNUInstallDirs)
include(CheckFunctionExists)

check_function_exists(personality HAVE_PERSONALITY)
check_function_exists(unshare HAVE_UNSHARE)
check_function_exists(setgroups HAVE_SETGROUPS)
configure_file(Doxyfile.in Doxyfile)
configure_file(config.hpp.in config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE true)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
set(PRODUCEIT_LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}/produceit")

file(GLOB scripts utils/*.rb)
file(GLOB bman1 src/*.1)
file(GLOB uman1 utils/*.1)
file(GLOB markdown *.md)
file(GLOB issues .gitlab/issue_templates/*.md)

add_subdirectory(src)

# these give access to docs and support files when cmake is exported to an ide

add_custom_target(manpages SOURCES ${bman1} ${uman1})
add_custom_target(scripts SOURCES ${scripts})
add_custom_target(support SOURCES produceit.conf config.hpp.in Doxyfile.in ${issues} .gitlab-ci.yml .rubocop.yml)
add_custom_target(docs SOURCES LICENSE ${markdown}
    COMMAND doxygen Doxyfile
)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/deb-release.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME deb-release )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/deb-archive.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME deb-archive )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/deb-remove.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME deb-remove )
install(FILES ${CMAKE_SOURCE_DIR}/produceit.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

install(FILES ${uman1} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

if(CMAKE_MAINTAINER)
    add_custom_target(dist
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.tar.gz"
        COMMAND git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_ARCHIVE}-${PROJECT_VERSION}/" "v${PROJECT_VERSION}" 2>/dev/null || git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_ARCHIVE}-${PROJECT_VERSION}/" HEAD   
    )

    add_custom_target(distclean
        COMMAND ${CMAKE_BUILD_TOOL} clean
        COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.tar.gz"
        COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.rpm"
        COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/*.pdf"
        COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/doc"
    )

    include("${CMAKE_CURRENT_SOURCE_DIR}/redhat/include.cmake" OPTIONAL)
    include("${CMAKE_CURRENT_SOURCE_DIR}/debian/include.cmake" OPTIONAL)
endif()

# if cross-compiling, can also generate cross-build packages...
if(CMAKE_CROSSCOMPILING)
    set(CPACK_SYSTEM_NAME "${PROJECT_RELEASE}.cross")
    set(CPACK_PACKAGE_NAME "${PROJECT_ARCHIVE}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "chroot package production and testing environment")
    set(CPACK_PACKAGE_VENDOR "Tycho Softworks")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_RPM_PACKAGE_URL "https://gitlab.com/tychosoft/produceit")
    set(CPACK_RPM_PACKAGE_RELEASE ${PROJECT_RELEASE})
    set(CPACK_RPM_PACKAGE_REQUIRES "rpm-build, ruby, gnupg2")
    set(CPACK_RPM_PACKAGE_SUGGESTS "qemu-user-static")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tycho Softworks")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "rpm, ruby, gnupg")
    include(CPack)
elseif(CMAKE_MAINTAINER)
    add_custom_target(pdf
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/latex"
        COMMAND make
        COMMAND mv refman.pdf "${CMAKE_CURRENT_BINARY_DIR}/produceit.pdf"
        DEPENDS docs
    )
endif()

# testing and coverage

if(CMAKE_COVERAGE_TYPE MATCHES "gcov")
    set(BUILD_TESTING true)
    add_custom_target(coverage
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND rm -f *.gcov
        COMMAND tests/test_output
        COMMAND gcov -b src/*.cpp -o src/CMakeFiles/common.dir
    )
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
