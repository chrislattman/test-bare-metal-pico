cmake_minimum_required(VERSION 3.27)

include(pico_sdk_import.cmake)

project(test-bare-metal-pico)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(blink_led blink_led.c)
target_link_libraries(blink_led
    pico_cyw43_arch_none
    pico_stdlib
    )

pico_enable_stdio_usb(blink_led 1)
pico_enable_stdio_uart(blink_led 1)
pico_add_extra_outputs(blink_led)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()
