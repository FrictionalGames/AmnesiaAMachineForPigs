cmake_minimum_required (VERSION 3.0)
project (AngelScript)

if (MSVC)
    enable_language(ASM_MASM)
endif()

set(AngelScript_VERSION_MAJOR 2)
set(AngelScript_VERSION_MINOR 19)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ASM_SORUCES)
if (MSVC)
  set(ASM_SORUCES
    sources/as_callfunc_x64_msvc_asm.asm)  # etc for all source files
    set_source_files_properties(${ASM_SORUCES} PROPERTIES LANGUAGE ASM_MASM)
    set_source_files_properties(${ASM_SORUCES} PROPERTIES COMPILE_DEFINITIONS  "_M_X64")
endif()

add_library(AngelScript STATIC
  ${ASM_SORUCES}
  sources/as_arrayobject.cpp
  sources/as_atomic.cpp
  # sources/as_callfunc_arm_msvc.asm
  sources/as_builder.cpp
  sources/as_bytecode.cpp
  # skip compiling assembly for arm
  # sources/as_callfunc_arm_gcc.S 
  # sources/as_callfunc_arm_xcode.s
  sources/as_callfunc_arm.cpp
  sources/as_callfunc_mips.cpp
  sources/as_callfunc_ppc_64.cpp
  sources/as_callfunc_ppc.cpp
  sources/as_callfunc_sh4.cpp
  sources/as_callfunc_x64_gcc.cpp
  sources/as_callfunc_x64_msvc.cpp
  sources/as_callfunc_x86.cpp
  sources/as_callfunc_xenon.cpp
  sources/as_callfunc.cpp
  sources/as_compiler.cpp
  sources/as_configgroup.cpp
  sources/as_context.cpp
  sources/as_datatype.cpp
  sources/as_gc.cpp
  sources/as_generic.cpp
  sources/as_globalproperty.cpp
  sources/as_memory.cpp
  sources/as_module.cpp
  sources/as_objecttype.cpp
  sources/as_outputbuffer.cpp
  sources/as_parser.cpp
  sources/as_restore.cpp
  sources/as_scriptcode.cpp
  sources/as_scriptengine.cpp
  sources/as_scriptfunction.cpp
  sources/as_scriptnode.cpp
  sources/as_scriptobject.cpp
  sources/as_string_util.cpp
  sources/as_string.cpp
  sources/as_thread.cpp
  sources/as_tokenizer.cpp
  sources/as_typeinfo.cpp
  sources/as_variablescope.cpp
  )

target_include_directories(AngelScript PUBLIC include)