cmake_minimum_required(VERSION 3.2)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

project(libc
    LANGUAGES C CXX ASM
)

include(ExternalProject)
ExternalProject_Add(compiler-rt-build
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/compiler-rt
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/compiler-rt
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/../../arm.toolchain.cmake"
        "-DCMAKE_C_FLAGS=\"-I${CMAKE_CURRENT_SOURCE_DIR}/include\""
        "-DCMAKE_ASM_FLAGS=\"-Ddmb=nop\"" #hack for sync_ primitives
        "-DHAVE_UNWIND_H=NO" #hack for gcc_personality_v0
        "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
        "-DCOMPILER_RT_OS_DIR=baremetal"
        "-DCOMPILER_RT_BAREMETAL_BUILD=ON"
        "-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON"
        "-DCOMPILER_RT_BUILD_SANITIZERS=OFF"
        "-DCOMPILER_RT_BUILD_XRAY=OFF"
        "-DCOMPILER_RT_BUILD_LIBFUZZER=OFF"
        "-DCOMPILER_RT_BUILD_PROFILE=OFF"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/compiler-rt/usr"
)
add_library(compiler-rt STATIC IMPORTED)
set_target_properties(compiler-rt PROPERTIES IMPORTED_LOCATION
    "${CMAKE_CURRENT_BINARY_DIR}/compiler-rt/usr/lib/baremetal/libclang_rt.builtins-arm.a")

add_subdirectory(lib)
