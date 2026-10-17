// Minimal stand-in for the Pico SDK's <pico.h> when building for the QEMU mps2-an521 machine.
// Only what hello_world.c needs is provided.
#ifndef QEMU_PICO_H
#define QEMU_PICO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PICO_OK 0
#define PICO_ERROR_GENERIC -1

// There is no flash or XIP on mps2-an521, so the upper 2 MB of the 4 MB code SSRAM stands in for it.
// The linker script only places the program in the first 2 MB.
#define XIP_BASE 0x10200000u
#define PICO_FLASH_SIZE_BYTES (1024u * 1024u * 2u)

// Same pin number as the Pico 2. The gpio functions are no-ops in QEMU.
#define PICO_DEFAULT_LED_PIN 25

#endif
