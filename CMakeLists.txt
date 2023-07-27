cmake_minimum_required(VERSION 3.21)

# compile_commands for c/c++ lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
set(PICO_BOARD "adafruit_feather_rp2040")

project(smallfry C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(C_STANDARD 23)
set(CXX_STANDARD 23)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

pico_sdk_init()

add_executable(smallfry src/smallfry.cc)
target_sources(smallfry PRIVATE src/smallfry.cc src/rgb8.cc src/rgb8.hh src/mfrc522.cc src/mfrc522.hh)

pico_generate_pio_header(smallfry ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

pico_enable_stdio_usb(smallfry 1)
pico_enable_stdio_uart(smallfry 1)

target_link_libraries(smallfry
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_pwm
    hardware_i2c
    hardware_spi
    pico_bootsel_via_double_reset
)

target_compile_definitions(smallfry PRIVATE
	PICO_BOOTSEL_VIA_DOUBLE_RESET_ACTIVITY_LED=13
    PICO_BOOTSEL_VIA_DOUBLE_RESET_TIMEOUT_MS=500
)

pico_add_extra_outputs(smallfry)

add_compile_options(-Ofast -Wall -Wextra -Wpedantic -flto)