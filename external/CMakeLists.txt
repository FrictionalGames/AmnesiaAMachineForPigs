include(FetchContent)


FetchContent_Declare(
  Eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(EIGEN_BUILD_DOC OFF)
set(EIGEN_BUILD_PKGCONFIG OFF)
FetchContent_MakeAvailable(Eigen)

add_subdirectory(OALWrapper)
add_subdirectory(Newton)
add_subdirectory(AngelScript)

set(THE_FORGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/The-Forge" CACHE STRING "Location of the forge.")

# file(GLOB EASTL_SOURCES
#    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/EASTL/*.cpp)
# add_library(Eastl STATIC ${EASTL_SOURCES})

add_library(cpu_features OBJECT 
     ${THE_FORGE_DIR}/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_macos.c
     ${THE_FORGE_DIR}/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_aarch64_iOS.c
     ${THE_FORGE_DIR}/Common_3/OS/ThirdParty/OpenSource/cpu_features/src/impl_x86_windows.c
)


file(GLOB BASIS_UNVERSAL_SOURCES 
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/*.c
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/transcoder/*.cpp
    #${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/*.cpp\

    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_astc_decomp.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_backend.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_basis_file.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_comp.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_enc.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_etc.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_frontend.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_global_selector_palette_helpers.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_gpu_texture.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_pvrtc1_4.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_resampler.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_resample_filters.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/basisu_ssim.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal/lodepng.cpp
)
add_library(basis_universal STATIC ${BASIS_UNVERSAL_SOURCES})
target_include_directories(basis_universal PUBLIC ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/basis_universal)

add_library(ags SHARED IMPORTED IMPORTED GLOBAL)
set_property(TARGET ags PROPERTY IMPORTED_LOCATION
         ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.dll
        )
set_property(TARGET ags PROPERTY IMPORTED_IMPLIB
         ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/ags/ags_lib/lib/amd_ags_x64.lib
        )
target_include_directories(ags INTERFACE  ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/ags)

# add_library(tinyEXR STATIC ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/TinyEXR/tinyexr.cc)
#target_include_directories(tinyEXR PRIVATE 
#    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/TinyEXR)

  
add_library(astc-encoder OBJECT 
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenccli_error_metrics.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenccli_image.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenccli_image_external.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenccli_image_load_store.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenccli_platform_dependents.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_averages_and_directions.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_block_sizes.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_color_quantize.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_color_unquantize.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_compress_symbolic.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_compute_variance.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_decompress_symbolic.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_diagnostic_trace.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_entry.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_find_best_partitioning.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_ideal_endpoints_and_weights.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_image.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_integer_sequence.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_mathlib.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_mathlib_softfloat.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_partition_tables.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_percentile_tables.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_pick_best_endpoint_format.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_platform_isa_detection.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_quantization.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_symbolic_physical.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_weight_align.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/astcenc_weight_quant_xfer_tables.cpp
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/astc-encoder/Source/wuffs-v0.3.c

    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/TinyEXR/miniz.c
)
target_include_directories(astc-encoder PRIVATE 
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/TinyEXR)
target_include_directories(astc-encoder  PUBLIC ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/zip)


# VulkanMemoryAllocator
add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator INTERFACE
    ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/VulkanMemoryAllocator)


# cglf
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/cgltf)

#BStr
add_library(Bstr STATIC 
    ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/bstrlib/bstrlib.c)
target_include_directories(Bstr PUBLIC ${THE_FORGE_DIR}/Common_3/Utilities/ThirdParty/OpenSource/bstrlib)

# add_library(volk STATIC 
#     ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/volk/volk.c)
# target_include_directories(volk PUBLIC ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/volk)

add_library(tinyimageformat INTERFACE)
target_include_directories(tinyimageformat INTERFACE 
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/tinyimageformat)

#file(GLOB OZZ_ANIMATION_SOURCES 
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/animation/offline/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/animation/runtime/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/base/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/base/containers/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/base/io/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/base/maths/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/base/memory/*.cc
#    ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/src/geometry/runtime/*.cc)
#
#add_library(ozz-animation STATIC ${OZZ_ANIMATION_SOURCES})
#target_include_directories(ozz-animation PUBLIC ${THE_FORGE_DIR}/Common_3/Resources/AnimationSystem/ThirdParty/OpenSource/ozz-animation/include)

file(GLOB MESH_OPTIMIZER_SOURCES ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src/*.cpp)
add_library(meshoptimizer STATIC ${MESH_OPTIMIZER_SOURCES})
target_include_directories(meshoptimizer PUBLIC ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/meshoptimizer/src)

add_library(tinydds INTERFACE)
target_include_directories(tinydds INTERFACE 
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/tinydds)

add_library(tinyktx INTERFACE)
target_include_directories(tinyktx INTERFACE 
    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/ThirdParty/OpenSource/tinyktx)

# ga input
file(GLOB GAINPUT_SOURCES 
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/builtin/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/dev/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/gestures/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/keyboard/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/mouse/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/pad/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/recorder/*.cpp
    )

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    file(GLOB GAINPUT_SOURCES_MACOS
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParts/OpenSource/gainput/lib/source/hidapi/mac/*.c
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParts/OpenSource/gainput/lib/source/gainput/*.mm)
    list(APPEND GAINPUT_SOURCES ${GAINPUT_SOURCES_MACOS})
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    file(GLOB GAINPUT_SOURCES_WINDOWS
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParts/OpenSource/gainput/lib/source/hidapi/windows/*.c
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/*.cpp
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/gainput/lib/source/gainput/hid/hidparsers/*.cpp
    )
    list(APPEND GAINPUT_SOURCES ${GAINPUT_SOURCES_WINDOWS})
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    file(GLOB GAINPUT_SOURCES_LINUX
        ${THE_FORGE_DIR}/Common_3/Application/ThirdParts/OpenSource/gainput/lib/source/hidapi/linux/*.c)
    list(APPEND GAINPUT_SOURCES ${GAINPUT_SOURCES_LINUX})
endif()

add_library(ga STATIC ${GAINPUT_SOURCES})
target_include_directories(ga PUBLIC ${THE_FORGE_DIR}/Common_3/ThirdParty/OpenSource/lua)



file(GLOB THE_FORGE_SOURCES
    ${THE_FORGE_DIR}/Common_3/Graphics/*.cpp
    ${THE_FORGE_DIR}/Common_3/Application/*.cpp
    ${THE_FORGE_DIR}/Common_3/Application/Profiler/*.cpp
    ${THE_FORGE_DIR}/Common_3/Application/Fonts/FontSystem.cpp
    ${THE_FORGE_DIR}/Common_3/Application/UI/UI.cpp
    ${THE_FORGE_DIR}/Common_3/Application/Profiler/*.cpp
    ${THE_FORGE_DIR}/Common_3/OS/WindowSystem/*.cpp
    ${THE_FORGE_DIR}/Common_3/OS/*.cpp

    ${THE_FORGE_DIR}/Common_3/Resources/ResourceLoader/*.cpp

    ${THE_FORGE_DIR}/Common_3/Utilities/FileSystem/FileSystem.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/FileSystem/SystemRun.cpp
    ${THE_FORGE_DIR}/Common_3/Utilities/Log/*.c
    ${THE_FORGE_DIR}/Common_3/Utilities/Math/*.c
    ${THE_FORGE_DIR}/Common_3/Utilities/MemoryTracking/*.c
    ${THE_FORGE_DIR}/Common_3/Utilities/Threading/*.c
    ${THE_FORGE_DIR}/Common_3/Utilities/*.c

    ${THE_FORGE_DIR}/Common_3/Application/InputSystem.cpp

    ${THE_FORGE_DIR}/Common_3/Game/Scripting/*.cpp
)

# volk
add_library(volk STATIC 
    ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/volk/volk.c)
if(WIN32)
    target_include_directories(volk  PUBLIC ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/Vulkan-Headers)
endif()
target_include_directories(volk PUBLIC ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/volk)

# SPIRV_Cross
file(GLOB SPIRV_CROSS_SOURCES ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/SPIRV_Cross/*.cpp)
add_library(SPIRV_Cross STATIC ${SPIRV_CROSS_SOURCES})
set_property(TARGET SPIRV_Cross PROPERTY CXX_STANDARD 17)

# imgui
file(GLOB IMGUI_SOURCES ${THE_FORGE_DIR}/Common_3/Application/ThirdParty/OpenSource/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${THE_FORGE_DIR}/Common_3/ThirdParty/OpenSource/imgui)


add_library(TheForge STATIC ${THE_FORGE_SOURCES})
target_link_libraries(TheForge PUBLIC cpu_features imgui astc-encoder cgltf Bstr basis_universal meshoptimizer tinydds tinyimageformat tinyktx ga)
target_include_directories(TheForge PUBLIC ${THE_FORGE_DIR})

IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    file(GLOB THE_FORGE_OS_DARWIN_SOURCES
        ${THE_FORGE_DIR}/Common_3/OS/Darwin/*.cpp
        ${THE_FORGE_DIR}/Common_3/OS/Darwin/*.c)
    target_sources(TheForge PRIVATE ${THE_FORGE_OS_DARWIN_SOURCES})
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    file(GLOB THE_FORGE_OS_WINDOWS_SOURCES
        ${THE_FORGE_DIR}/Common_3/OS/Windows/*.cpp
        ${THE_FORGE_DIR}/Common_3/OS/Windows/*.c)
        
    target_sources(TheForge PRIVATE ${THE_FORGE_OS_WINDOWS_SOURCES})
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    include(FindPkgConfig)

    file(GLOB THE_FORGE_OS_LINUX_SOURCES
        ${THE_FORGE_DIR}/Common_3/OS/Linux/*.cpp
        ${THE_FORGE_DIR}/Common_3/OS/Linux/*.c
        ${THE_FORGE_DIR}/Common_3/Utilities/FileSystem/UnixFileSystem.cpp)
    target_sources(TheForge PRIVATE ${THE_FORGE_OS_LINUX_SOURCES})

    pkg_check_modules(GTK REQUIRED gtk+-3.0)

    target_link_libraries(TheForge PRIVATE ${GTK_LIBRARIES})
    target_include_directories(TheForge PRIVATE ${GTK_INCLUDE_DIRS})
endif()

file(GLOB GRAPHICS_VULKAN_SOURCE
    "${THE_FORGE_DIR}/Common_3/Graphics/Vulkan/*.cpp")
target_sources(TheForge PRIVATE ${GRAPHICS_VULKAN_SOURCE})
target_link_libraries(TheForge PRIVATE SPIRV_Cross volk)

# WinPixEventRuntime
add_library(WinPixEventRuntime SHARED IMPORTED GLOBAL)
set_property(TARGET WinPixEventRuntime PROPERTY IMPORTED_LOCATION
        ${THE_FORGE_DIR}/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.dll)
set_property(TARGET WinPixEventRuntime PROPERTY IMPORTED_IMPLIB
        ${THE_FORGE_DIR}/Common_3/OS/ThirdParty/OpenSource/winpixeventruntime/bin/WinPixEventRuntime.lib)

# DirectXShaderCompiler 
add_library(DirectXShaderCompiler SHARED IMPORTED GLOBAL)
set_property(TARGET DirectXShaderCompiler PROPERTY IMPORTED_LOCATION
        ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/bin/x64/dxcompiler.dll)
set_property(TARGET DirectXShaderCompiler PROPERTY IMPORTED_IMPLIB
    ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/lib/x64/dxcompiler.lib)     


# Nvapi
add_library(Nvapi STATIC IMPORTED)
set_property(TARGET Nvapi PROPERTY IMPORTED_LOCATION
    ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/nvapi/amd64/nvapi64.lib
    )

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    file(GLOB GRAPHICS_D3D12_SOURCE
        "${THE_FORGE_DIR}/Common_3/Graphics/Direct3D12/*.cpp")
    target_sources(TheForge PRIVATE ${GRAPHICS_D3D12_SOURCE})
    target_include_directories(TheForge PRIVATE ${THE_FORGE_DIR}/Common_3/Graphics/ThirdParty/OpenSource/D3D12MemoryAllocator)
    target_link_libraries(TheForge PRIVATE Nvapi ags)

    file(GLOB GRAPHICS_D3D11_SOURCE "${THE_FORGE_DIR}/Common_3/Graphics/Direct3D11/*.cpp")
    target_sources(TheForge PRIVATE ${GRAPHICS_D3D11_SOURCE})
    target_link_libraries(TheForge PRIVATE DirectXShaderCompiler "d3d11.lib")
    target_link_libraries(TheForge PRIVATE WinPixEventRuntime)
endif()


set(HPL_3RDPARTY_TARGETS
	Newton
	AngelScript
	OALWrapper
    TheForge
)

SET(FSL_COMPILER  "${THE_FORGE_DIR}/Common_3/Tools/ForgeShadingLanguage/fsl.py" PARENT_SCOPE)
set(_HPL_3RDPARTY_TARGETS_ ${HPL_3RDPARTY_TARGETS} PARENT_SCOPE)
