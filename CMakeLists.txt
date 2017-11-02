cmake_minimum_required(VERSION 3.1.0)
project(produceit VERSION 0.1.0 LANGUAGES CXX)
set(PROJECT_RELEASE "1")

include(${CMAKE_CURRENT_SOURCE_DIR}/Custom.cmake OPTIONAL)
include(GNUInstallDirs)
include(CheckFunctionExists)

check_function_exists(personality HAVE_PERSONALITY)
check_function_exists(unshare HAVE_UNSHARE)
check_function_exists(setgroups HAVE_SETGROUPS)
configure_file(Doxyfile.in Doxyfile)
configure_file(src/config.hpp.in config.hpp NEWLINE_STYLE UNIX)
include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE true)
set(PRODUCEIT_LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}/produceit")

file(GLOB scripts utils/*.rb)
file(GLOB markdown *.md)

add_subdirectory(src)

# these give access to docs and support files when cmake is exported to an ide

add_custom_target(support SOURCES produceit.conf produceit.spec ${scripts} src/config.hpp.in Doxyfile.in)

add_custom_target(docs SOURCES LICENSE ${markdown}
    COMMAND doxygen Doxyfile
)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/dsc-archive.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME dsc-archive )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/dsc-remove.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME dsc-remove )
install(FILES ${CMAKE_SOURCE_DIR}/produceit.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

add_custom_target(dist
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-*.tar.gz"
    COMMAND git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_NAME}-${PROJECT_VERSION}/" "v${PROJECT_VERSION}" 2>/dev/null || git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_NAME}-${PROJECT_VERSION}/" HEAD   
)

add_custom_target(source
    DEPENDS dist
)

add_custom_target(distclean
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-*.tar.gz"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-*.rpm"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/*.pdf"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/doc"
)

# if cross-compiling, can also generate cross-build packages...
if(CMAKE_CROSSCOMPILING)
    set(CPACK_SYSTEM_NAME "${PROJECT_RELEASE}.cross")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
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
elseif(UNIX)
    add_custom_target(srpm
        DEPENDS dist
        COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-*.rpm"
        COMMAND rpmbuild -bs --nodeps "${CMAKE_CURRENT_SOURCE_DIR}/produceit.spec"
    )

    add_custom_target(pdf
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/latex"
        COMMAND make
        COMMAND mv refman.pdf "${CMAKE_CURRENT_BINARY_DIR}/produceit.pdf"
        DEPENDS docs
    )
endif()

