// Startup code for the QEMU mps2-an521 machine (Cortex-M33, secure state).
// Sets up the C runtime, then calls main(). Replaces the Pico SDK's crt0.S and runtime_init.
#include <stdint.h>
#include <string.h>

#define SCB_AIRCR (*(volatile uint32_t *)0xE000ED0C)

extern uint32_t __etext, __data_start__, __data_end__, __bss_start__, __bss_end__, __StackTop;

extern int main(void);
extern void __libc_init_array(void);
void SysTick_Handler(void);
void NMI_Handler(void);

void Reset_Handler(void);

static void Default_Handler(void) {
    while (1) {
    }
}

void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".vectors"), used))
static void (*const vector_table[16])(void) = {
    (void (*)(void))&__StackTop,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    Default_Handler, // MemManage
    Default_Handler, // BusFault
    Default_Handler, // UsageFault
    Default_Handler, // SecureFault
    0, 0, 0,
    Default_Handler, // SVCall
    Default_Handler, // DebugMonitor
    0,
    Default_Handler, // PendSV
    SysTick_Handler,
};

void system_reset(void) {
    __asm volatile("dsb");
    SCB_AIRCR = (0x05FAu << 16) | (1u << 2); // VECTKEY | SYSRESETREQ
    __asm volatile("dsb");
    while (1) {
    }
}

void Reset_Handler(void) {
    memcpy(&__data_start__, &__etext, (uintptr_t)&__data_end__ - (uintptr_t)&__data_start__);
    memset(&__bss_start__, 0, (uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__);

    __libc_init_array();
    main();
    while (1) {
        __asm volatile("wfi");
    }
}

// newlib calls _init/_fini from __libc_init_array/exit
void _init(void) {}
void _fini(void) {}
