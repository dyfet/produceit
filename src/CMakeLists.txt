file(GLOB headers *.hpp)
set(sources args.cpp keyfile.cpp fsys.cpp dir.cpp mount.cpp strings.cpp proc.cpp)

add_library(common STATIC ${headers} ${sources})

add_executable(buildit buildit.cpp)
add_dependencies(buildit common)
target_link_libraries(buildit common)

add_executable(shellit shellit.cpp)
add_dependencies(shellit common)
target_link_libraries(shellit common)

install(TARGETS buildit shellit DESTINATION ${CMAKE_INSTALL_BINDIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)

