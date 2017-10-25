cmake_minimum_required(VERSION 3.1.0)
project(ProduceIt VERSION 0.1.0 LANGUAGES CXX)
string(TOLOWER "${PROJECT_NAME}" PROJECT_ARCHIVE)

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

add_custom_target(support SOURCES produceit.conf produceit.spec ${scripts} src/config.hpp.in Doxyfile.in)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/dsc-archive.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME dsc-archive )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/dsc-remove.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME dsc-remove )
install(FILES ${CMAKE_SOURCE_DIR}/produceit.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

add_custom_target(dist
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.tar.gz"
    COMMAND git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_ARCHIVE}-${PROJECT_VERSION}/" "v${PROJECT_VERSION}" 2>/dev/null || git archive -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-${PROJECT_VERSION}.tar.gz" --format tar.gz --prefix="${PROJECT_ARCHIVE}-${PROJECT_VERSION}/" HEAD   
)

add_custom_target(srpm
    DEPENDS dist
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.rpm"
    COMMAND rpmbuild -bs --nodeps "${CMAKE_CURRENT_SOURCE_DIR}/produceit.spec"
)

add_custom_target(docs SOURCES LICENSE ${markdown}
    COMMAND doxygen Doxyfile
)

add_custom_target(pdf
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/latex"
    COMMAND make
    COMMAND mv refman.pdf "${CMAKE_CURRENT_BINARY_DIR}/produceit.pdf"
    DEPENDS docs
)

add_custom_target(distclean
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.tar.gz"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_ARCHIVE}-*.rpm"
    COMMAND "${CMAKE_COMMAND}" -E remove -F "${CMAKE_CURRENT_BINARY_DIR}/*.pdf"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/doc"
)

