cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "Set PICO_SDK_PATH to a Raspberry Pi Pico SDK checkout")
endif()

include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(programming_lambda_keypad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "RP2040 board definition")
pico_sdk_init()

add_executable(programming_lambda_keypad
    main.c
    usb_descriptors.c
)

target_include_directories(programming_lambda_keypad PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(programming_lambda_keypad PRIVATE
    hardware_gpio
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(programming_lambda_keypad 0)
pico_enable_stdio_uart(programming_lambda_keypad 0)
pico_add_extra_outputs(programming_lambda_keypad)
