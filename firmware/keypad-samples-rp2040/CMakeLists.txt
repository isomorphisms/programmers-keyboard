cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "Set PICO_SDK_PATH to a Raspberry Pi Pico SDK checkout")
endif()

include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(keypad_samples_rp2040 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "RP2040 board definition")
pico_sdk_init()

function(add_keypad_sample target layout_id)
    add_executable(${target}
        main.c
        usb_descriptors.c
    )

    target_compile_definitions(${target} PRIVATE KEYPAD_LAYOUT=${layout_id})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(${target} PRIVATE
        hardware_gpio
        pico_stdlib
        pico_unique_id
        tinyusb_device
        tinyusb_board
    )

    pico_enable_stdio_usb(${target} 0)
    pico_enable_stdio_uart(${target} 0)
    pico_add_extra_outputs(${target})
endfunction()

add_keypad_sample(concept_separation_keypad 1)
add_keypad_sample(incantation_keypad 2)
add_keypad_sample(math_keypad 3)
add_keypad_sample(regex_keypad 4)
add_keypad_sample(pastebins_keypad 5)
add_keypad_sample(signals_keypad 6)
