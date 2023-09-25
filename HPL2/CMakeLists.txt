cmake_minimum_required (VERSION 3.0)
project(HPL2)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(HPL2_VERSION_MAJOR 2)
set(HPL2_VERSION_MINOR 0)

add_definitions(
  -DUSE_OALWRAPPER
  -DUSE_SDL2
)

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
    source/windowing/*
    sources/sound/*
    sources/system/*
    source/platform/sdl2/*

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
    sources/impl/scriptstring.cpp
    sources/impl/scriptstring_utils.cpp
    # Image loading
    sources/impl/BitmapLoader*
    # Newton
    sources/impl/*Newton.cpp
    sources/impl/LegacyVertexBuffer.cpp
    # SDL
    sources/impl/GamepadSDL.cpp
    sources/impl/GamepadSDL2.cpp
    sources/impl/KeyboardSDL.cpp
    sources/impl/MouseSDL.cpp
    sources/impl/TimerSDL.cpp
    sources/impl/LowLevelGraphicsSDL.cpp
    sources/impl/LowLevelInputSDL.cpp
    sources/impl/LowLevelResourcesSDL.cpp
    sources/impl/LowLevelSystemSDL.cpp
    sources/impl/SDLEngineSetup.cpp
    sources/impl/SDLFontData.cpp
    sources/impl/SDLTexture.cpp

    sources/platform/sdl2/*.cpp
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
ELSEIF(WIN32)
    LIST(APPEND impl_sources
        sources/impl/PlatformWin32.cpp
    )
ELSEIF(LINUX)
    LIST(APPEND impl_sources
        sources/impl/PlatformUnix.cpp
        sources/impl/PlatformSDL.cpp
    )
ENDIF()

# Setup buildID source

IF(APPLE)
    SET(BUILDID_SUFFIX MacOSX)
ELSEIF(LINUX)
    SET(BUILDID_SUFFIX Linux)
ELSEIF(WIN32)
    SET(BUILDID_SUFFIX Win32)
ELSE()
    message(FATAL_ERROR "Unknown platform")
ENDIF()
SET(BUILDID_SUFFIX ${BUILDID_SUFFIX} CACHE STRING "Build ID Suffix")
find_file(BUILDID_BIN buildcounter.pl
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}
    NO_DEFAULT_PATH
)
mark_as_advanced(BUILDID_SUFFIX BUILDID_BIN)

function(GenerateBuildID outvar idname includedir sourcedir)
    add_custom_command(
        OUTPUT
            ${sourcedir}/BuildID_${idname}_${BUILDID_SUFFIX}.cpp
            ${includedir}/BuildID_${idname}.h
        COMMAND
            perl ${BUILDID_BIN}
        ARGS
            ${idname} ${includedir} ${sourcedir}
    )
    set(${outvar} ${sourcedir}/BuildID_${idname}_${BUILDID_SUFFIX}.cpp
            ${includedir}/BuildID_${idname}.h PARENT_SCOPE)
endfunction()

GenerateBuildID(version_source
    HPL2_0
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sources
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ResourcesLocation.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ResourcesLocation.h)

# setup source groups
source_group("AI" REGULAR_EXPRESSION "\\/ai\\/.+")
source_group("Engine" REGULAR_EXPRESSION "\\/engine\\/.+")
source_group("Generate" REGULAR_EXPRESSION "\\/generate\\/.+")
source_group("Graphics" REGULAR_EXPRESSION "\\/graphics\\/.+")
source_group("GUI" REGULAR_EXPRESSION "\\/gui\\/.+")
source_group("Input" REGULAR_EXPRESSION "\\/input\\/.+")
source_group("Math" REGULAR_EXPRESSION "\\/math\\/.+")
source_group("Physics" REGULAR_EXPRESSION "\\/physics\\/.+")
source_group("Resources" REGULAR_EXPRESSION "\\/resources\\/.+")
source_group("Scene" REGULAR_EXPRESSION "\\/scene\\/.+")
source_group("Sound" REGULAR_EXPRESSION "\\/sound\\/.+")
source_group("System" REGULAR_EXPRESSION "\\/system\\/.+")

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

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(DevIL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
# find_package(PNG REQUIRED)
# find_package (Python COMPONENTS Interpreter)
# find_package(Python3 COMPONENTS Interpreter)

# setup HPL2 compile target

add_library(HPL2 STATIC
    ${common_sources}
    ${version_sources}
    ${impl_sources}
)

set(HPL2_INCLUDES
    PUBLIC include
    PUBLIC ${SDL2_INCLUDE_DIR}
    #Important that this is before dependencies so the correct OpenAL headers are used
    PRIVATE ${OALWRAPPER_INCLUDE_DIR}
    PRIVATE ${ZLIB_INCLUDE_DIRS}
    PRIVATE ${IL_INCLUDE_DIR}
)

set(RENDERDOC_PATH "/usr/local" CACHE PATH "Path to RenderDoc.")
set(RENDERDOC_ENABLED OFF CACHE BOOL "Enable RenderDoc integration. Use RENDERDOC_PATH to specific the path to RenderDoc.")

if(RENDERDOC_ENABLED)
    add_definitions(-DHPL2_RENDERDOC_ENABLED)
    set(HPL2_INCLUDES
        ${HPL2_INCLUDES}
        PRIVATE ${RENDERDOC_PATH}
    )
endif()


target_include_directories(HPL2
    ${HPL2_INCLUDES}
)

if(LINUX)
  set(PLATFORM_LIBS
    pthread
    dl
    )
endif()

# Don't use exceptions
if(WIN32)
  add_compile_options(/EHs-c-)
else()
  add_compile_options(-fno-exceptions)
endif()

target_link_libraries(HPL2
  OALWrapper # also pulls in ogg,vorbis,SDL
  AngelScript
  Newton
  Folly::folly 
  Folly::folly_deps
  TheForge
  Eigen3::Eigen
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  ZLIB::ZLIB
  ${IL_LIBRARIES}
  ${PLATFORM_LIBS}
  )
 

  file(GLOB shader_resources RELATIVE 
    "${CMAKE_CURRENT_SOURCE_DIR}/resource/*.sc"
    "${CMAKE_CURRENT_SOURCE_DIR}/resource/*.io"
    "${CMAKE_CURRENT_SOURCE_DIR}/resource/*.sh")



set(HPL2_SHADER ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)
set(HPL2_COMPILEDSHADER ${CMAKE_CURRENT_BINARY_DIR}/CompiledShaders/)
get_filename_component(SHADER_RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resource" ABSOLUTE)

add_custom_target(HPL_SHADERS)
IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # omit for now Vulkan is not an option 
    # add_custom_command(
    #     TARGET HPL_SHADERS
    #     MAIN_DEPENDENCY
    #     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    #     COMMAND ${CMAKE_COMMAND} -E env "VULKAN_SDK=$ENV{VULKAN_SDK}" "python3" "${FSL_COMPILER}"
    #     -l VULKAN 
    #     -d ${HPL2_SHADER}
    #     --verbose 
    #     -b  ${HPL2_COMPILEDSHADER}
    #     --incremental 
    #     --compile ${SHADER_RESOURCE_PATH}/ShaderList.fsl
    # )
    add_custom_command(
        TARGET HPL_SHADERS
        MAIN_DEPENDENCY
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env "VULKAN_SDK=$ENV{VULKAN_SDK}" "python3" "${FSL_COMPILER}"
        -l DIRECT3D12 
        -d ${HPL2_SHADER}
        --verbose 
        -b  ${HPL2_COMPILEDSHADER}
        --incremental 
        --compile ${SHADER_RESOURCE_PATH}/ShaderList.fsl
    )
       
else()
    add_custom_command(
        TARGET HPL_SHADERS
        MAIN_DEPENDENCY
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env "VULKAN_SDK=$ENV{VULKAN_SDK}" "python3" "${FSL_COMPILER}"
        -l VULKAN 
        -d ${HPL2_SHADER}
        --verbose 
        -b ${HPL2_COMPILEDSHADER}
        --incremental 
        --compile ${SHADER_RESOURCE_PATH}/ShaderList.fsl
    )
endif()

function(copy_shader_to_target_dir TARGET) 
    add_custom_target(${TARGET}_HPL_SHADERS)
    IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
        add_custom_command(
            TARGET ${TARGET}_HPL_SHADERS
            MAIN_DEPENDENCY
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E env "VULKAN_SDK=$ENV{VULKAN_SDK}" "python3" "${FSL_COMPILER}"
            -l DIRECT3D12 
            -d "$<TARGET_FILE_DIR:${TARGET}>/Shaders"
            --verbose 
            -b  "$<TARGET_FILE_DIR:${TARGET}>/CompiledShaders"
            --incremental 
            --compile ${_SHADER_RESOURCE_PATH_}/ShaderList.fsl
        )
           
    else()
        add_custom_command(
            TARGET ${TARGET}_HPL_SHADERS
            MAIN_DEPENDENCY
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E env "VULKAN_SDK=$ENV{VULKAN_SDK}" "python3" "${FSL_COMPILER}"
            -l VULKAN 
            -d "$<TARGET_FILE_DIR:${TARGET}>/Shaders"
            --verbose 
            -b  "$<TARGET_FILE_DIR:${TARGET}>/CompiledShaders"
            --incremental 
            --compile ${_SHADER_RESOURCE_PATH_}/ShaderList.fsl
        )
    endif()
    add_dependencies(HPL2 ${TARGET}_HPL_SHADERS)
endfunction()


set(_HPL2_SHADER_ ${HPL2_SHADER} PARENT_SCOPE)
set(_HPL2_COMPILEDSHADER_ ${HPL2_COMPILEDSHADER} PARENT_SCOPE)
set(_SHADER_RESOURCE_PATH_  ${SHADER_RESOURCE_PATH} PARENT_SCOPE)
 add_dependencies(HPL2 HPL_SHADERS)
