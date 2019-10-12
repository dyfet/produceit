if(UNIX)
    if(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_COMPILE_SETUID_FLAGS)
        set(CMAKE_COMPILE_SETUID_FLAGS "-O2 -fPIE -fstack-protector -D_FORTIFY_SOURCE=2 --param ssp-buffer-size=4 -pie")
        if(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
            set(CMAKE_LINK_SETUID_FLAGS "-pie")
        else()
            set(CMAKE_LINK_SETUID_FLAGS "-pie -z relro -z now")
        endif()
    endif()
endif()

file(GLOB headers *.hpp)
set(sources args.cpp keyfile.cpp fsys.cpp dir.cpp mount.cpp strings.cpp output.cpp proc.cpp)

add_library(common STATIC ${headers} ${sources})

add_executable(buildit buildit.cpp)
add_dependencies(buildit common)
target_link_libraries(buildit common)

add_executable(shellit shellit.cpp)
add_dependencies(shellit common)
target_link_libraries(shellit common)

if(LINUX)
    add_executable(lxcuser lxcuser.cpp)
    add_dependencies(lxcuser common)
    target_link_libraries(lxcuser common pthread)
endif()

if(UNIX)
    set_target_properties(buildit PROPERTIES COMPILE_FLAGS "${CMAKE_COMPILE_SETUID_FLAGS}" LINK_FLAGS "${CMAKE_LINK_SETUID_FLAGS}" POSITION_INDEPENDENT_CODE TRUE )
    set_target_properties(shellit PROPERTIES COMPILE_FLAGS "${CMAKE_COMPILE_SETUID_FLAGS}" LINK_FLAGS "${CMAKE_LINK_SETUID_FLAGS}" POSITION_INDEPENDENT_CODE TRUE )
    set_target_properties(common PROPERTIES COMPILE_FLAGS "${CMAKE_COMPILE_SETUID_FLAGS}" LINK_FLAGS "${CMAKE_LINK_SETUID_FLAGS}" POSITION_INDEPENDENT_CODE TRUE )
    if(LINUX)
        set_target_properties(lxcuser PROPERTIES COMPILE_FLAGS "${CMAKE_COMPILE_SETUID_FLAGS}" LINK_FLAGS "${CMAKE_LINK_SETUID_FLAGS}" POSITION_INDEPENDENT_CODE TRUE )
    endif()
endif()

install(TARGETS buildit shellit DESTINATION ${CMAKE_INSTALL_BINDIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)

install(FILES buildit.1 shellit.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

if(LINUX)
    install(TARGETS lxcuser DESTINATION ${CMAKE_INSTALL_BINDIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID)
    install(FILES lxcuser.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()
