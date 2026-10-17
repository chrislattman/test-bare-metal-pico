// Minimal stand-in for the Pico SDK's <hardware/watchdog.h> when building for the QEMU mps2-an521 machine.
// Backed by the SSE-200 secure CMSDK watchdog.
#ifndef QEMU_HARDWARE_WATCHDOG_H
#define QEMU_HARDWARE_WATCHDOG_H

#include <pico.h>

void watchdog_enable(uint32_t delay_ms, bool pause_on_debug);
void watchdog_update(void);

#endif
