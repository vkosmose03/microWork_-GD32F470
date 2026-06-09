cmake_minimum_required(VERSION 3.20)

project(gd32f470_acmeter
    VERSION 1.0.0
    DESCRIPTION "AC voltage measurement firmware for GD32F470 (RMS / frequency / dF/dt)"
    HOMEPAGE_URL "https://github.com/"
    LANGUAGES C ASM)

# ----------------------------------------------------------------------------
# Language standard: C99 as required by the task (req #1).
# ----------------------------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

# ----------------------------------------------------------------------------
# Strict warning policy. -Werror directly enforces requirement #7: the project
# must build without errors *or warnings*. These flags are applied ONLY to our
# own code (via the acmeter_warnings interface target), never to the vendored
# GigaDevice/CMSIS sources, which live behind their own non-strict target.
# ----------------------------------------------------------------------------
add_library(acmeter_warnings INTERFACE)
target_compile_options(acmeter_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wshadow
    -Wundef
    -Wdouble-promotion   # catch accidental double math on the single-precision FPU
    -Wstrict-prototypes
    -Wmissing-prototypes)

# ----------------------------------------------------------------------------
# Platform-independent core: calculation, measurement and configuration logic.
# Compiles unchanged for both the host (unit tests) and the GD32F470 target,
# which is what makes the modules unit-testable (req* unit tests, req #5).
# It depends ONLY on the abstract HAL headers in src/hal, never on vendor code.
# ----------------------------------------------------------------------------
add_library(acmeter_core STATIC
    src/calc/rms.c
    src/calc/freq.c
    src/calc/dfdt.c
    src/calc/harmonic.c
    src/calc/signal_calc.c
    src/measure/measure.c)
target_include_directories(acmeter_core PUBLIC src)
target_link_libraries(acmeter_core PUBLIC acmeter_warnings)
# Single-precision math (sqrtf, sinf, ...) keeps the FPU busy and the CPU free.
target_link_libraries(acmeter_core PUBLIC m)

if(CMAKE_CROSSCOMPILING)
    # ------------------------------------------------------------------------
    # TARGET build: assemble the firmware image for the GD32F470.
    # ------------------------------------------------------------------------
    message(STATUS "Configuring firmware build for GD32F470 target")
    add_subdirectory(bsp/gd32f470)

    add_executable(firmware
        src/app/main.c
        src/app/app.c)
    target_link_libraries(firmware PRIVATE acmeter_core gd32_bsp acmeter_warnings)
    set_target_properties(firmware PROPERTIES SUFFIX ".elf")

    target_link_options(firmware PRIVATE
        -T${CMAKE_CURRENT_SOURCE_DIR}/bsp/gd32f470/gd32f470zg_flash.ld
        -Wl,-Map=$<TARGET_FILE_DIR:firmware>/firmware.map,--cref
        -Wl,--gc-sections
        -Wl,--print-memory-usage
        --specs=nano.specs
        --specs=nosys.specs)

    # Produce .bin/.hex and print the image size after every build.
    add_custom_command(TARGET firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware>
                $<TARGET_FILE_DIR:firmware>/firmware.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:firmware>
                $<TARGET_FILE_DIR:firmware>/firmware.hex
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
        COMMENT "Generating firmware.bin / firmware.hex and size report")
else()
    # ------------------------------------------------------------------------
    # HOST build: unit tests for the platform-independent modules.
    # ------------------------------------------------------------------------
    message(STATUS "Configuring host build (unit tests)")
    option(ENABLE_COVERAGE "Build host tests with gcov coverage instrumentation" OFF)
    if(ENABLE_COVERAGE)
        target_compile_options(acmeter_core PRIVATE --coverage -O0 -g)
        target_link_options(acmeter_core PUBLIC --coverage)
    endif()

    enable_testing()
    add_subdirectory(tests)
endif()
