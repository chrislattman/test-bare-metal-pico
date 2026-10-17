/* Linker script for the QEMU mps2-an521 machine (Arm SSE-200, Cortex-M33), running in secure state.
 *
 * QEMU's mps2-an521 memory map (secure aliases):
 *   0x10000000 - 0x103FFFFF  ZBT SSRAM1 (4 MB)    used as "flash": code, read-only data, and the
 *                                                 emulated XIP flash region (see qemu/include/pico.h)
 *   0x38000000 - 0x383FFFFF  ZBT SSRAM2/3 (4 MB)  used as RAM: data, bss, heap, stack
 *
 * The CPU loads its initial SP and PC from the secure vector table at 0x10000000 (INITSVTOR0).
 */

MEMORY
{
    CODE (rx)  : ORIGIN = 0x10000000, LENGTH = 2M
    RAM  (rwx) : ORIGIN = 0x38000000, LENGTH = 4M
}

ENTRY(Reset_Handler)

__stack_size = 0x10000;

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
    } > CODE

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > CODE

    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > CODE

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > CODE

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > CODE

    __etext = ALIGN(4);

    .data : AT(__etext)
    {
        __data_start__ = .;
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM

    /* LMA = VMA keeps .bss out of the .data load segment, whose LMA is in CODE: QEMU's ELF loader
     * zero-fills a segment's memory size at its LMA, which would run past the end of CODE */
    .bss (NOLOAD) : AT(ADDR(.bss))
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    /* Heap from the end of .bss up to the stack, which grows down from the top of RAM */
    end = __bss_end__;
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    __HeapLimit = __StackTop - __stack_size;
    ASSERT(__HeapLimit >= end, "RAM overflow")
}
