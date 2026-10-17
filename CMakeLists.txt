cmake_minimum_required(VERSION 3.27)

# Must come before pico_sdk_import.cmake, which otherwise defaults the build type to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Build type" FORCE)
endif()

include(pico_sdk_import.cmake)

project(test-bare-metal-pico)

set(CMAKE_C_FLAGS_DEBUG "-Og -ggdb3")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(hello_world hello_world.c)
target_link_libraries(hello_world pico_stdlib)
if(PICO_BOARD STREQUAL pico2_w)
target_link_libraries(hello_world pico_cyw43_arch_none)
endif()

pico_enable_stdio_usb(hello_world 1)
pico_enable_stdio_uart(hello_world 1)
pico_add_extra_outputs(hello_world)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

add_custom_target(run
    COMMAND "${CMAKE_BINARY_DIR}/_deps/picotool/picotool" load -u -v -x "${CMAKE_BINARY_DIR}/hello_world.elf"
    DEPENDS hello_world
    USES_TERMINAL
    VERBATIM
)

# QEMU has no RP2350 machine, so the same hello_world.c is also built for mps2-an521 (also a
# Cortex-M33, so the SDK's compiler flags mostly still apply). It doesn't link the Pico SDK: the
# headers in qemu/include stand in for the SDK headers, implemented on the AN521 peripherals in qemu/.
add_executable(hello_world_qemu hello_world.c qemu/startup.c qemu/pico_shim.c)
target_include_directories(hello_world_qemu PRIVATE qemu/include)
# QEMU's AN521 CPU has no FPU, so override the SDK's -march=...+fp so that no FP instructions are
# generated (this also selects newlib's no-FP multilib at link time)
set(QEMU_ARCH_FLAGS -mcpu=cortex-m33+nofp -march=armv8-m.main+dsp)
target_compile_options(hello_world_qemu PRIVATE ${QEMU_ARCH_FLAGS})
target_link_options(hello_world_qemu PRIVATE ${QEMU_ARCH_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/qemu/mps2_an521.ld -nostartfiles --specs=nano.specs -Wl,--gc-sections)
set_target_properties(hello_world_qemu PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qemu/mps2_an521.ld)

set(QEMU_ELF $<TARGET_FILE:hello_world_qemu>)

# printf output (UART0) goes to the serial port. In another shell: telnet localhost 5678
# Quit qemu: (qemu) quit
# Quit telnet: Ctrl + ] -> telnet> quit
add_custom_target(qemu
    COMMAND qemu-system-arm -machine mps2-an521 -kernel ${QEMU_ELF} -nographic
            -serial telnet:localhost:5678,server=on,wait=off
    DEPENDS hello_world_qemu
    USES_TERMINAL
    VERBATIM
)

# Same as qemu, but halts at startup and waits for GDB on localhost:1234
# (replace -s with -gdb tcp:localhost:<PORT> to use a different port, use with -DCMAKE_BUILD_TYPE=Debug).
# In another shell:
#   gdb-multiarch -q build/hello_world_qemu.elf
#   (gdb) target remote localhost:1234
#   (gdb) break main
#   (gdb) continue
# In another shell: telnet localhost 5678
add_custom_target(qemu_debug
    COMMAND qemu-system-arm -machine mps2-an521 -kernel ${QEMU_ELF} -nographic
            -serial telnet:localhost:5678,server=on,wait=off -S -s
    DEPENDS hello_world_qemu
    USES_TERMINAL
    VERBATIM
)
