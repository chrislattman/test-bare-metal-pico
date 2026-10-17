// Minimal stand-in for the Pico SDK's <pico/flash.h> when building for the QEMU mps2-an521 machine.
#ifndef QEMU_PICO_FLASH_H
#define QEMU_PICO_FLASH_H

#include <pico.h>

int flash_safe_execute(void (*func)(void *), void *param, uint32_t enter_exit_timeout_ms);

#endif
