cmake_minimum_required(VERSION 3.1.0)
project(ProduceIt VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckFunctionExists)

check_function_exists(personality HAVE_PERSONALITY)
check_function_exists(unshare HAVE_UNSHARE)
check_function_exists(setgroups HAVE_SETGROUPS)
configure_file(src/config.hpp.in config.hpp NEWLINE_STYLE UNIX)
configure_file(utils/test-shellit.sh test-shellit COPYONLY)
include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE true)
set(PRODUCEIT_LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}/produceit")
set(common_sources src/args.cpp src/keyfile.cpp src/fsys.cpp src/dir.cpp src/mount.cpp src/strings.cpp src/proc.cpp)
file(GLOB common_headers src/*.hpp)
file(GLOB utils_ruby utils/*.rb)
file(GLOB utils_shell utils/*.sh)

source_group(Common FILES ${common_headers} ${common_sources})
source_group(Utils FILES ${utils_ruby} ${utils_shell})

add_library(common STATIC ${common_headers} ${common_sources})

add_executable(buildit src/buildit.cpp)
add_dependencies(buildit common)
target_link_libraries(buildit common)

add_executable(shellit src/shellit.cpp)
add_dependencies(shellit common)
target_link_libraries(shellit common)

install(TARGETS buildit shellit DESTINATION ${CMAKE_INSTALL_BINDIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/dsc-archive.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME dsc-archive )
install(PROGRAMS ${CMAKE_SOURCE_DIR}/utils/dsc-remove.rb DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME dsc-remove )
install(FILES ${CMAKE_SOURCE_DIR}/produceit.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})

