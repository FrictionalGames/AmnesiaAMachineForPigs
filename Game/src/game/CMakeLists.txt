cmake_minimum_required (VERSION 2.8)
project(Amnesia)

include_directories(
	${HPL2_INCLUDE_DIR}
	${DEP_INCLUDE_DIR}
)

link_directories(
    ${DEP_LIB_DIR}
)

file(GLOB all_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.cpp
    *.h
)
file(GLOB excluded_source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    LuxProp_OilBarrel.cpp
    LuxInsanityHandler.cpp
)
list(REMOVE_ITEM all_sources ${excluded_source})

if (APPLE)
    set(mac_assets
        aamfp.icns
    )
    list(APPEND all_sources
        ${mac_assets}
    )
    set_source_files_properties(${mac_assets}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
    )
endif()

set(compile_defs "")
set(demo_compile_defs "LUX_DEMO_VERSION")

list(APPEND compile_defs ${HPL2_DEFINES})

option(USE_GAMEPAD "Enable gamepad support" ON)
if(USE_GAMEPAD)
	list(APPEND compile_defs USE_GAMEPAD)
	list(APPEND demo_compile_defs USE_GAMEPAD)
endif()

set(full_sources ${all_sources})

add_executable(Amnesia MACOSX_BUNDLE
    ${full_sources}
)
target_link_libraries(Amnesia HPL2)
set_target_properties(Amnesia PROPERTIES
	OUTPUT_NAME "AmnesiaAMFP"
	COMPILE_DEFINITIONS "${compile_defs}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Game-Info.plist"
	MACOSX_BUNDLE_ICON_FILE "aamfp.icns"
	MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
	MACOSX_BUNDLE_GUI_IDENTIFIER "com.frictionalgames.aamfp-game"
	MACOSX_BUNDLE_DISPLAY_NAME "AmnesiaAMFPLauncher"
    MACOSX_BUNDLE_BUNDLE_NAME "Amnesia: A Machine for Pigs"
)

add_executable(AmnesiaDemo MACOSX_BUNDLE EXCLUDE_FROM_ALL
    ${all_sources}
)
target_link_libraries(AmnesiaDemo HPL2)
set_target_properties(AmnesiaDemo PROPERTIES
	OUTPUT_NAME "AmnesiaAMFPDemo"
	COMPILE_DEFINITIONS "${demo_compile_defs}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Game-Info.plist"
	MACOSX_BUNDLE_ICON_FILE "aamfp.icns"
	MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}-demo"
	MACOSX_BUNDLE_GUI_IDENTIFIER "com.frictionalgames.aamfp-game-demo"
    MACOSX_BUNDLE_BUNDLE_NAME "Amnesia: A Machine for Pigs - Demo"
)

IF(APPLE)
	PostBuildMacBundle(Amnesia "${HPL2_FRAMEWORKS}" "${HPL2_LIBRARIES}")
	PostBuildMacBundle(AmnesiaDemo "${HPL2_FRAMEWORKS}" "${HPL2_LIBRARIES}")
	install(TARGETS Amnesia
		RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../../redist"
		BUNDLE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../../redist"
	)
ENDIF()

# vim: noet ts=4:
