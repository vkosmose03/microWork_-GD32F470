# Board-support package for the GD32F470. This is the ONLY place the GigaDevice
# firmware library is used. The vendor sources are fetched at configure time and
# compiled into a separate, non-strict-warning target so that the strict
# -Werror policy applies to our code without being broken by third-party code.
#
# For offline / local iteration, point at a local checkout with:
#   -DFETCHCONTENT_SOURCE_DIR_GD32F4XX=/path/to/gd32f4xx_firmware_library
include(FetchContent)

FetchContent_Declare(
    gd32f4xx
    GIT_REPOSITORY https://github.com/cjacker/gd32f4xx_firmware_library_gcc_makefile.git
    GIT_TAG 3b5d2f265fd40cd39993e4b12fdaeee6bd7aeb65
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(gd32f4xx)

set(GD32_FW "${gd32f4xx_SOURCE_DIR}/Firmware")

# ---- Vendor library (CMSIS + standard peripheral + startup) -----------------
# Compile only the peripheral units this firmware uses. This keeps the image
# small and avoids unrelated modules (CAN/DBG/ENET/...) that would need extra
# headers enabled in gd32f4xx_libopt.h.
set(GD32_SPL_MODULES rcu gpio syscfg dma adc timer usart fmc pmu misc)
set(GD32_SPL_SOURCES "")
foreach(m IN LISTS GD32_SPL_MODULES)
    list(APPEND GD32_SPL_SOURCES
        "${GD32_FW}/GD32F4xx_standard_peripheral/Source/gd32f4xx_${m}.c")
endforeach()

add_library(gd32_vendor STATIC
    ${GD32_SPL_SOURCES}
    "${GD32_FW}/CMSIS/GD/GD32F4xx/Source/system_gd32f4xx.c"
    "${GD32_FW}/CMSIS/GD/GD32F4xx/Source/GCC/startup_gd32f450_470.S")

target_include_directories(gd32_vendor PUBLIC
    "${GD32_FW}/CMSIS"
    "${GD32_FW}/CMSIS/GD/GD32F4xx/Include"
    "${GD32_FW}/GD32F4xx_standard_peripheral/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}") # for gd32f4xx_libopt.h

# Select the target device; this drives the register/IRQ definitions.
target_compile_definitions(gd32_vendor PUBLIC GD32F470)
# Third-party code: do not subject it to our strict warning policy.
target_compile_options(gd32_vendor PRIVATE -w)

# ---- Our board-support drivers (implement the HAL) --------------------------
add_library(gd32_bsp STATIC
    hal_time_gd32.c
    hal_uart_gd32.c
    hal_nvm_gd32.c
    hal_adc_gd32.c
    syscalls.c)
target_include_directories(gd32_bsp PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(gd32_bsp PUBLIC gd32_vendor)
# First-party BSP code: warnings are errors, but without the pedantic/undef set
# that the vendor headers would trip over.
target_compile_options(gd32_bsp PRIVATE -Wall -Wextra -Werror)
