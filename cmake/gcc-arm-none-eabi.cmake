# CMake toolchain file for the GD32F470 (Arm Cortex-M4F) using the
# GNU Arm Embedded toolchain (arm-none-eabi-gcc).
#
# Usage:
#   cmake -S . -B build-fw -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
#   cmake --build build-fw

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy CACHE FILEPATH "objcopy")
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size    CACHE FILEPATH "size")

# GD32F470 = Cortex-M4 with a single-precision hardware FPU (FPv4-SP-D16).
set(CPU_OPTIONS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
add_compile_options(${CPU_OPTIONS}
    -ffunction-sections
    -fdata-sections
    -fno-common)
add_link_options(${CPU_OPTIONS})

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")

# Do not try to link a full executable when probing the compiler (there is no
# linker script / startup code available during the compiler check).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
