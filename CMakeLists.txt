cmake_minimum_required (VERSION 2.8)
IF(APPLE)
#    SET(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvmgcc42")
    SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.6)
ENDIF()
project(Amnesia)

SET(VERSION "1.0.3")

OPTION(USERDIR_RESOURCES "Allow using local userdirectory resources" OFF)

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
    SET(LINUX ON)
ENDIF()

OPTION(FORCE32 "Force a 32bit compile on 64bit" OFF)
if (APPLE)
    SET(FORCE32 ON)
endif()

IF(FORCE32)
    if(APPLE)
        SET(CMAKE_OSX_ARCHITECTURES "i386")
    else()
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    endif()
ENDIF()

IF(LINUX)
    if(CMAKE_SIZEOF_VOID_P MATCHES "8" AND NOT(FORCE32) )
        set(BIN_LIBROOT "linux64")
        set(CMAKE_EXECUTABLE_SUFFIX ".bin.x86_64")
        set(BIN_RPATH "\$ORIGIN/lib64")
    else()
        set(BIN_LIBROOT "linux")
        set(CMAKE_EXECUTABLE_SUFFIX ".bin.x86")
        set(BIN_RPATH "\$ORIGIN/lib")
    endif()

    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH ${BIN_RPATH})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_CXX_FLAGS "-fpermissive -no-pie")
ELSEIF(APPLE)
    SET(BIN_LIBROOT "macosx")
    SET(BIN_RPATH "@executable_path/../Frameworks")

    set(CMAKE_SKIP_BUILD_RPATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH ${BIN_RPATH})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
ENDIF()

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

# silence some annoying warnings
add_definitions(
    -fno-strict-aliasing
    -Wno-switch
    -Wno-reorder
    -Wno-unused-variable
    -Wno-unused-function
)

set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2013 Frictional Games")

add_subdirectory(HPL2/core core)

if(USERDIR_RESOURCES)
    add_definitions(
        -DUSERDIR_RESOURCES
    )
endif()

add_subdirectory(Game/src/game game)
IF(APPLE)
    add_subdirectory(Game/src/launcher-macosx launcher)
ELSE()
    add_subdirectory(Game/src/launcher launcher)
ENDIF()

add_custom_target(BuildGame)
add_dependencies(BuildGame Amnesia Launcher)

add_custom_Target(BuildSteamGame)
add_dependencies(BuildSteamGame AmnesiaSteam Launcher)

