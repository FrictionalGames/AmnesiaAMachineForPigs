cmake_minimum_required (VERSION 2.8)
project(HPL2)

set(HPL2_VERSION_MAJOR 2)
set(HPL2_VERSION_MINOR 0)

set(HPL2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
set(DEP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/include" PARENT_SCOPE)
set(DEP_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/lib/${BIN_LIBROOT}")
set(DEP_LIB_DIR ${DEP_LIB_DIR} PARENT_SCOPE)

IF(APPLE)
    SET(PRIVATE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies)
    SET(PRIVATE_LIB_SUFFIX "lib/macosx")
ELSEIF(LINUX)
    SET(PRIVATE_LIB_SUFFIX "lib/${BIN_LIBROOT}")
    SET(PRIVATE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies)
ENDIF()

OPTION(USE_SDL2 "Use SDL2 instead of SDL1.2" ON)

add_subdirectory(../dependencies/OALWrapper OALWrapper)

IF(USE_SDL2)
    add_definitions(-DUSE_SDL2)
    SET(HPL2_DEFINES USE_SDL2 PARENT_SCOPE)
ENDIF()

add_definitions(
    -DUSE_OALWRAPPER
)

include_directories(
    include
    #Important that this is before dependencies so the correct OpenAL headers are used
    ${OALWRAPPER_INCLUDE_DIR}
    ../dependencies/include
)

IF(LINUX)
    add_library(binreloc STATIC ../dependencies/sources/binreloc/binreloc.c)
    set_target_properties(binreloc PROPERTIES COMPILE_DEFINITIONS "ENABLE_BINRELOC")

    include_directories(
        ../dependencies/sources/binreloc/
    )
ENDIF()

# common shared source
file(GLOB common_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    sources/ai/*
    sources/engine/*
    sources/generate/*
    sources/graphics/*
    sources/gui/*
    sources/haptic/*
    sources/input/*
    sources/math/*
    sources/physics/*
    sources/resources/*
    sources/scene/*
    sources/sound/*
    sources/system/*

    include/ai/*
    include/engine/*
    include/generate/*
    include/graphics/*
    include/gui/*
    include/haptic/*
    include/input/*
    include/math/*
    include/physics/*
    include/resources/*
    include/scene/*
    include/sound/*
    include/system/*
    include/*.*
)
list(APPEND common_sources include/BuildID_HPL2_0.h)
list(REMOVE_DUPLICATES common_sources)

#setup various IMPL sources
file(GLOB impl_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    include/impl/tinyXML/*
    include/impl/*.*

    # tinyXML
    sources/impl/tinyXml/*
    sources/impl/XmlDocumentTiny.cpp
    # scripting
    sources/impl/SqScript.cpp
    sources/impl/scriptarray.cpp
    sources/impl/scripthelper.cpp
    sources/impl/scriptstdstring.cpp
    sources/impl/scriptstdstring_utils.cpp
    # Image loading
    sources/impl/BitmapLoader*
    # theora video
    sources/impl/VideoStreamTheora.cpp
    # Newton
    sources/impl/*Newton.cpp
    # GL
    sources/impl/FrameBufferGL.cpp
    sources/impl/GLSL*
    sources/impl/OcclusionQueryOGL.cpp
    sources/impl/VertexBufferOGL_Array.cpp
    sources/impl/VertexBufferOGL_VBO.cpp
    sources/impl/VertexBufferOpenGL.cpp
    # SDL
    sources/impl/GamepadSDL.cpp
    sources/impl/GamepadSDL2.cpp
    sources/impl/KeyboardSDL.cpp
    sources/impl/MouseSDL.cpp
    sources/impl/MutexSDL.cpp
    sources/impl/ThreadSDL.cpp
    sources/impl/TimerSDL.cpp
    sources/impl/LowLevelGraphicsSDL.cpp
    sources/impl/LowLevelInputSDL.cpp
    sources/impl/LowLevelResourcesSDL.cpp
    sources/impl/LowLevelSystemSDL.cpp
    sources/impl/SDLEngineSetup.cpp
    sources/impl/SDLFontData.cpp
    sources/impl/SDLTexture.cpp
    # OpenAL
    sources/impl/LowLevelSoundOpenAL.cpp
    sources/impl/OpenAL*
    # mesh loader
    sources/impl/MeshLoaderCollada.cpp
    sources/impl/MeshLoaderColladaHelpers.cpp
    sources/impl/MeshLoaderColladaLoader.cpp
    sources/impl/MeshLoaderMSH.cpp
    sources/impl/MeshLoaderFBX.cpp
)

IF(APPLE)
    LIST(APPEND impl_sources
        sources/impl/PlatformMacOSX.mm
        sources/impl/PlatformUnix.cpp
        sources/impl/PlatformSDL.cpp
    )
    IF(NOT USE_SDL2)
        LIST(APPEND impl_sources
            SDLMain.m
            SDLMain.h
        )
    ENDIF()
ELSEIF(LINUX)
    LIST(APPEND impl_sources
        sources/impl/PlatformUnix.cpp
        sources/impl/PlatformSDL.cpp
    )
ENDIF()

# Setup buildID source

IF(APPLE)
    SET(VERSION_SUFFIX MacOSX)
ELSEIF(LINUX)
    SET(VERSION_SUFFIX Linux)
ENDIF()

SET(version_source
    sources/BuildID_HPL2_0_${VERSION_SUFFIX}.cpp
)

set_property(
    SOURCE
        sources/BuildID_HPL2_0_${VERSION_SUFFIX}.cpp
        include/BuildID_HPL2_0.h
    PROPERTY
        GENERATED)

set(BUILDCOUNTER_BIN "${CMAKE_CURRENT_SOURCE_DIR}/buildcounter.pl")
set(BUILDCOUNTER_BIN ${BUILDCOUNTER_BIN} PARENT_SCOPE)

add_custom_command(
    OUTPUT
        sources/BuildID_HPL2_0_${VERSION_SUFFIX}.cpp
        include/BuildID_HPL2_0.h
    COMMAND
        ${BUILDCOUNTER_BIN}
    ARGS
        HPL2_0 ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/sources
)

# setup source groups
source_group("Implementation Specific" REGULAR_EXPRESSION "\\/impl\\/.+")
source_group("TinyXML" REGULAR_EXPRESSION "\\/impl\\/tiny.+")
source_group("SDL Specific" REGULAR_EXPRESSION "\\/impl\\/.*SDL.+")
source_group("OpenGL Specific" REGULAR_EXPRESSION "\\/impl\\/.*GL.+")
source_group("HaptX Specific" REGULAR_EXPRESSION "\\/impl\\/.+HaptX.+")
source_group("Newton Specific" REGULAR_EXPRESSION "\\/impl\\/.+Newton.+")
source_group("Mesh Loaders" REGULAR_EXPRESSION "\\/impl\\/Mesh.+")
source_group("OpenAL Specific" REGULAR_EXPRESSION "\\/impl\\/.*OpenAL.+")
source_group("Win32" REGULAR_EXPRESSION "\\/impl\\/.+Win32.+")
source_group("Platform" REGULAR_EXPRESSION "\\/impl\\/Platform.+")


# setup libs

add_library(Newton STATIC IMPORTED)
set_target_properties(Newton PROPERTIES IMPORTED_LOCATION ${DEP_LIB_DIR}/libNewton.a)

add_library(angelscript STATIC IMPORTED)
set_target_properties(angelscript PROPERTIES IMPORTED_LOCATION ${DEP_LIB_DIR}/libangelscript.a)

add_library(FBX STATIC IMPORTED)
set_target_properties(FBX PROPERTIES IMPORTED_LOCATION ${DEP_LIB_DIR}/libfbxsdk-2012.2-static.a)

# setup HPL2 compile target

add_library(HPL2 STATIC
    ${common_sources}
    ${version_sources}
    ${impl_sources}
)

if (LINUX)
    target_link_libraries(HPL2
        OALWrapper # also pulls in ogg,vorbis,SDL
        Newton angelscript binreloc
        GL GLU GLEW
        theora
        IL
        z
        pthread dl
        FBX
    )
ELSEIF(APPLE)
    FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
    FIND_LIBRARY(SC_LIBRARY SystemConfiguration)
    FIND_LIBRARY(THEORA_LIBRARY Theora PATHS ${DEP_LIB_DIR} NO_DEFAULT_PATH)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    target_link_libraries(HPL2
        OALWrapper
        GLEW
        z
        Newton
        angelscript
        IL
        FBX iconv ${SC_LIBRARY}
        ${THEORA_LIBRARY}
        ${OPENGL_LIBRARY}
        ${COCOA_LIBRARY}
    )
    
    # helper function to copy the needed frameworks in at build time
    SET(HPL2_LIBRARIES libSDL2.dylib libIL.1.dylib)
    SET(HPL2_LIBRARIES ${HPL2_LIBRARIES} PARENT_SCOPE)

    FUNCTION(PostBuildMacBundle APP_TARGET framework_list lib_list)
        INCLUDE(BundleUtilities)
        GET_TARGET_PROPERTY(_BIN_NAME ${APP_TARGET} LOCATION)
        GET_DOTAPP_DIR(${_BIN_NAME} _BUNDLE_DIR)

        set(_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${APP_TARGET}_prep.cmake)
        file(WRITE ${_SCRIPT_FILE}
            "# Generated Script file\n"
            "include(BundleUtilities)\n"
            "get_bundle_and_executable(\"\${BUNDLE_APP}\" bundle executable valid)\n"
            "if(valid)\n"
            "  set(framework_path \"\${bundle}/Contents/Frameworks\")\n"
            "  foreach(framework ${framework_list})\n"
            "    file(MAKE_DIRECTORY \"\${framework_path}/\${framework}.framework/Versions/A/\")\n"
            "    copy_resolved_framework_into_bundle(\${DEP_LIB_DIR}/\${framework}.framework/Versions/A/\${framework} \${framework_path}/\${framework}.framework/Versions/A/\${framework})\n"
            "  endforeach()\n"
            "  foreach(lib ${lib_list})\n"
            "    copy_resolved_item_into_bundle(\${DEP_LIB_DIR}/\${lib} \${framework_path}/\${lib})\n"
            "  endforeach()\n"
            "else()\n"
            "  message(ERROR \"App Not found? \${BUNDLE_APP}\")\n"
            "endif()\n"
            "#fixup_bundle(\"\${BUNDLE_APP}\" \"\" \"\${DEP_LIB_DIR}\")\n"
        )

        ADD_CUSTOM_COMMAND(TARGET ${APP_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DBUNDLE_APP="${_BUNDLE_DIR}" -DDEP_LIB_DIR="${DEP_LIB_DIR}" -P "${_SCRIPT_FILE}"
        )
    ENDFUNCTION()
ENDIF()

FUNCTION(AddTestTarget target_name)
    add_executable(${target_name} EXCLUDE_FROM_ALL
        ${ARGN}
    )
    target_link_libraries(${target_name} HPL2)
    set_target_properties(${target_name} PROPERTIES
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${DEP_LIB_DIR}"
        INSTALL_RPATH "${DEP_LIB_DIR}"
    )
ENDFUNCTION()

add_subdirectory(../tools/editors editors)
add_subdirectory(../tools/mshconverter mshconverter)

