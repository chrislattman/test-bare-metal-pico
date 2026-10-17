// Minimal stand-in for the Pico SDK's <hardware/flash.h> when building for the QEMU mps2-an521 machine.
// "Flash" is the RAM region at XIP_BASE, so erase and program are plain memory writes.
#ifndef QEMU_HARDWARE_FLASH_H
#define QEMU_HARDWARE_FLASH_H

#include <pico.h>

#define FLASH_PAGE_SIZE (1u << 8)
#define FLASH_SECTOR_SIZE (1u << 12)
#define FLASH_BLOCK_SIZE (1u << 16)

void flash_range_erase(uint32_t flash_offs, size_t count);
void flash_range_program(uint32_t flash_offs, const uint8_t *data, size_t count);

#endif
