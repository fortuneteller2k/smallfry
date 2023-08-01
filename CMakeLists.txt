cmake_minimum_required(VERSION 3.21)
set(PROJECT_NAME smallfry)

# compile_commands for c/c++ lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FreeRTOS_Kernel_import.cmake)

set(PICO_BOARD "adafruit_feather_rp2040")

project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(C_STANDARD 23)
set(CXX_STANDARD 23)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

pico_sdk_init()

add_executable(${PROJECT_NAME} src/smallfry.cc)
target_sources(${PROJECT_NAME} PRIVATE
    src/smallfry.cc
    src/rtos.hh
    src/rgb8.cc
    src/rgb8.hh
    src/mfrc522.cc
    src/mfrc522.hh
)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_pwm
    hardware_i2c
    hardware_spi
    pico_bootsel_via_double_reset
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	PICO_BOOTSEL_VIA_DOUBLE_RESET_ACTIVITY_LED=13
    PICO_BOOTSEL_VIA_DOUBLE_RESET_TIMEOUT_MS=500
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    $ENV{FREERTOS_KERNEL_PATH}/include
    #${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/GCC/ARM_CM0
)

pico_add_extra_outputs(${PROJECT_NAME})

add_compile_options(-Ofast -Wall -Wextra -Wpedantic -flto)