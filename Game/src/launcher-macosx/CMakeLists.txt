cmake_minimum_required (VERSION 2.8.11)
project(LauncherOSX)

SET(HPL2_SOURCE_DIR ${HPL2_INCLUDE_DIR}/../sources)

include_directories(
    ${HPL2_INCLUDE_DIR}
    ${DEP_INCLUDE_DIR}
)

link_directories(
    ${DEP_LIB_DIR}
)

set(compile_defs "HPL_MINIMAL")
list(APPEND compile_defs ${HPL2_DEFINES})

FILE(GLOB HPL2_MINIMAL_SOURCES
    ${HPL2_SOURCE_DIR}/impl/PlatformUnix.cpp
    ${HPL2_SOURCE_DIR}/impl/PlatformSDL.cpp
    ${HPL2_SOURCE_DIR}/impl/PlatformMacOSX.mm
    ${HPL2_INCLUDE_DIR}/resources/ConfigFile.h
    ${HPL2_SOURCE_DIR}/resources/ConfigFile.cpp
    ${HPL2_INCLUDE_DIR}/resources/BinaryBuffer.h
    ${HPL2_SOURCE_DIR}/resources/BinaryBuffer.cpp
    ${HPL2_INCLUDE_DIR}/resources/LanguageFile.h
    ${HPL2_SOURCE_DIR}/resources/LanguageFile.cpp
    ${HPL2_INCLUDE_DIR}/graphics/Color.h
    ${HPL2_SOURCE_DIR}/graphics/Color.cpp
    ${HPL2_INCLUDE_DIR}/graphics/GraphicsTypes.h
    ${HPL2_SOURCE_DIR}/graphics/GraphicsTypes.cpp
    ${HPL2_INCLUDE_DIR}/math/CRC.h
    ${HPL2_SOURCE_DIR}/math/CRC.cpp
    ${HPL2_INCLUDE_DIR}/system/Platform.h
    ${HPL2_SOURCE_DIR}/system/Platform.cpp
    ${HPL2_INCLUDE_DIR}/system/String.h
    ${HPL2_SOURCE_DIR}/system/String.cpp
    ${HPL2_SOURCE_DIR}/impl/tinyXml/*
#    ${HPL2_SOURCE_DIR}/impl/XmlDocumentTiny.cpp
    HPLMinimal.h
    HPLMinimal.mm
)

add_library(HPL2_Minimal
    ${HPL2_MINIMAL_SOURCES}
)
set_target_properties(HPL2_Minimal PROPERTIES
    COMPILE_DEFINITIONS "${compile_defs}"
)
target_link_libraries(HPL2_Minimal
    z
)

SET(RESOURCE_FILES
    MainWindow.xib
    Startup.plist
    ../game/aamfp.icns
)
SET(BACKGROUND_FILE
    ../launcher/background.jpg
)
add_executable(Launcher MACOSX_BUNDLE
    ../launcher/Qualitychooser.h
    ../launcher/Qualitychooser.cpp
    LauncherAppDelegate.h
    LauncherAppDelegate.mm
    TabBackground.h
    TabBackground.m
    main.m
    Launcher_Prefix.pch
    ${RESOURCE_FILES}
    ${BACKGROUND_FILE}
)
set_source_files_properties(${RESOURCE_FILES} ${BACKGROUND_FILE}
    PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
)
set_target_properties(Launcher PROPERTIES
    OUTPUT_NAME "AmnesiaAMFPLauncher"
    COMPILE_DEFINITIONS "${compile_defs}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Launcher-Info.plist"
    MACOSX_BUNDLE_ICON_FILE "aamfp.icns"
    MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.frictionalgames.aamfp"
    MACOSX_BUNDLE_BUNDLE_NAME "Amnesia: A Machine for Pigs"
)

FIND_LIBRARY(COCOA_LIBRARY Cocoa)
FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
FIND_LIBRARY(OPENAL_LIBRARY OpenAL)
FIND_LIBRARY(SDL2_LIBRARY SDL2
    PATHS ${DEP_LIB_DIR}
    NO_DEFAULT_PATH
)
MARK_AS_ADVANCED(COCOA_LIBRARY OPENGL_LIBRARY OPENAL_LIBRARY SDL_LIBRARY)

target_link_libraries(Launcher
    ${COCOA_LIBRARY}
    ${OPENGL_LIBRARY}
    ${OPENAL_LIBRARY}
    ${SDL_LIBRARY}
    HPL2_Minimal
)

IF(APPLE)
	PostBuildMacBundle(Launcher "" "libSDL2.dylib")
	install(TARGETS Launcher
		RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../../redist"
		BUNDLE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../../redist"
	)
ENDIF()

# vim: et ts=4
