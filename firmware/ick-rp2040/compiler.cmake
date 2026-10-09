# Include before project(): Pico keeps its existing C++/ASM/runtime toolchain.
if(NOT DEFINED ICK_RP2040 AND DEFINED ENV{ICK_RP2040})
    set(ICK_RP2040 "$ENV{ICK_RP2040}")
endif()
if(NOT IS_ABSOLUTE "${ICK_RP2040}" OR NOT EXISTS "${ICK_RP2040}")
    message(FATAL_ERROR "Set ICK_RP2040 to the qualified arm-none-eabi ICK driver")
endif()
execute_process(COMMAND "${ICK_RP2040}" -dumpmachine
    OUTPUT_VARIABLE ick_target OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE ick_status)
if(NOT ick_status EQUAL 0 OR NOT ick_target STREQUAL "arm-none-eabi")
    message(FATAL_ERROR "RP2040 requires the bare-metal arm-none-eabi ICK target")
endif()
execute_process(COMMAND "${ICK_RP2040}" -print-file-name=include
    OUTPUT_VARIABLE ick_include OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE ick_status)
if(NOT ick_status EQUAL 0 OR NOT EXISTS "${ick_include}/stdint.h")
    message(FATAL_ERROR "The selected ICK stage lacks its own resource headers")
endif()
if(NOT DEFINED ICK_NEWLIB_INCLUDE)
    set(ICK_NEWLIB_INCLUDE "/usr/include/newlib")
endif()
if(NOT EXISTS "${ICK_NEWLIB_INCLUDE}/stdio.h")
    message(FATAL_ERROR "Set ICK_NEWLIB_INCLUDE to the embedded toolchain's newlib headers")
endif()
if(DEFINED CMAKE_C_COMPILER AND NOT CMAKE_C_COMPILER STREQUAL ICK_RP2040)
    message(FATAL_ERROR "Configure in a clean build directory with the selected ICK compiler")
endif()
set(CMAKE_C_COMPILER "${ICK_RP2040}" CACHE FILEPATH "Qualified RP2040 ICK C compiler")
add_compile_options(
    "$<$<COMPILE_LANGUAGE:C>:-nostdinc>"
    "$<$<COMPILE_LANGUAGE:C>:-isystem${ick_include}>"
    "$<$<COMPILE_LANGUAGE:C>:-isystem${ICK_NEWLIB_INCLUDE}>")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ICK_RP2040 ICK_NEWLIB_INCLUDE)
