set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TARGET_TRIPLE arm-none-eabi)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER clang)
set(CMAKE_ASM_COMPILER_TARGET ${TARGET_TRIPLE})

set(TOOLCHAIN_COMPILER_FLAGS "-march=armv7a -mcpu=arm926ej-s -mbig-endian -mfloat-abi=softfp -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TOOLCHAIN_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOOLCHAIN_COMPILER_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${TOOLCHAIN_COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TOOLCHAIN_COMPILER_FLAGS}")

# an extremely odd hack for compiler-rt to work
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
