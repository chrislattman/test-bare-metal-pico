// Implementations of the Pico SDK functions used by hello_world.c, on top of the peripherals that
// QEMU models for the mps2-an521 machine.
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <hardware/flash.h>
#include <hardware/watchdog.h>
#include <pico/flash.h>
#include <pico/stdlib.h>

// QEMU clocks the AN521 CPU (and so SysTick and the watchdogs) at 20 MHz
#define SYSCLK_HZ 20000000u

// SysTick (secure)
#define SYST_CSR (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018)
#define SYST_RELOAD (SYSCLK_HZ / 1000u - 1u) // 1 ms tick

// CMSDK APB UART0 (secure alias), connected to QEMU's first -serial
#define UART0_BASE 0x50200000u
#define UART_DATA (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_STATE (*(volatile uint32_t *)(UART0_BASE + 0x04))
#define UART_CTRL (*(volatile uint32_t *)(UART0_BASE + 0x08))
#define UART_BAUDDIV (*(volatile uint32_t *)(UART0_BASE + 0x10))
#define UART_STATE_TXFULL (1u << 0)
#define UART_CTRL_TXEN (1u << 0)

// CMSDK APB watchdog (secure), its interrupt is routed to NMI
#define WDOG_BASE 0x50081000u
#define WDOG_LOAD (*(volatile uint32_t *)(WDOG_BASE + 0x000))
#define WDOG_CTRL (*(volatile uint32_t *)(WDOG_BASE + 0x008))
#define WDOG_INTCLR (*(volatile uint32_t *)(WDOG_BASE + 0x00C))
#define WDOG_LOCK (*(volatile uint32_t *)(WDOG_BASE + 0xC00))
#define WDOG_UNLOCK_KEY 0x1ACCE551u
#define WDOG_CTRL_INTEN (1u << 0)

void system_reset(void);

static volatile uint32_t ms_ticks;

static struct {
    volatile bool active;
    alarm_callback_t callback;
    void *user_data;
    uint64_t target_us;
} alarm;

// ---------------------------------------------------------------------------------------------- stdio

bool stdio_init_all(void) {
    UART_BAUDDIV = SYSCLK_HZ / 115200u; // QEMU ignores the rate, but drops output if BAUDDIV < 16
    UART_CTRL = UART_CTRL_TXEN;

    SYST_RVR = SYST_RELOAD;
    SYST_CVR = 0;
    SYST_CSR = 0x7; // processor clock, interrupt enabled, counter enabled
    return true;
}

static void uart_putc(char c) {
    while (UART_STATE & UART_STATE_TXFULL) {
    }
    UART_DATA = (uint8_t)c;
}

// newlib syscalls used by printf
int _write(int fd, const char *buf, int len) {
    (void)fd;
    for (int i = 0; i < len; i++) {
        uart_putc(buf[i]);
    }
    return len;
}

void *_sbrk(ptrdiff_t incr) {
    extern char end, __HeapLimit;
    static char *heap_end = &end;
    char *prev = heap_end;
    if (heap_end + incr > &__HeapLimit) {
        errno = ENOMEM;
        return (void *)-1;
    }
    heap_end += incr;
    return prev;
}

int _close(int fd) { (void)fd; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { (void)fd; return 1; }
int _lseek(int fd, int ptr, int dir) { (void)fd; (void)ptr; (void)dir; return 0; }
int _read(int fd, char *buf, int len) { (void)fd; (void)buf; (void)len; return 0; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
void _exit(int status) { (void)status; while (1) {} }

// ----------------------------------------------------------------------------------------------- gpio

// There's nothing to see in QEMU, so these do nothing
void gpio_init(unsigned int gpio) { (void)gpio; }

void gpio_set_dir(unsigned int gpio, bool out) { (void)gpio; (void)out; }

void gpio_put(unsigned int gpio, bool value) { (void)gpio; (void)value; }

// ----------------------------------------------------------------------------------------------- time

uint64_t time_us_64(void) {
    uint32_t ms, cvr;
    do {
        ms = ms_ticks;
        cvr = SYST_CVR;
    } while (ms != ms_ticks);
    return (uint64_t)ms * 1000u + (SYST_RELOAD - cvr) / (SYSCLK_HZ / 1000000u);
}

void sleep_us(uint64_t us) {
    uint64_t target = time_us_64() + us;
    while (time_us_64() < target) {
        __asm volatile("wfi"); // woken by the next SysTick, so QEMU doesn't spin the host CPU
    }
}

void sleep_ms(uint32_t ms) { sleep_us(ms * 1000ull); }

alarm_id_t add_alarm_in_us(uint64_t us, alarm_callback_t callback, void *user_data, bool fire_if_past) {
    (void)fire_if_past;
    alarm.callback = callback;
    alarm.user_data = user_data;
    alarm.target_us = time_us_64() + us;
    alarm.active = true;
    return 1;
}

void SysTick_Handler(void) {
    ms_ticks++;
    if (alarm.active && time_us_64() >= alarm.target_us) {
        int64_t next = alarm.callback(1, alarm.user_data);
        if (next > 0) {
            alarm.target_us = time_us_64() + (uint64_t)next;
        } else if (next < 0) {
            alarm.target_us -= (uint64_t)next;
        } else {
            alarm.active = false;
        }
    }
}

// ---------------------------------------------------------------------------------------------- flash

int flash_safe_execute(void (*func)(void *), void *param, uint32_t enter_exit_timeout_ms) {
    (void)enter_exit_timeout_ms;
    func(param);
    return PICO_OK;
}

void flash_range_erase(uint32_t flash_offs, size_t count) {
    memset((void *)(XIP_BASE + flash_offs), 0xFF, count);
}

void flash_range_program(uint32_t flash_offs, const uint8_t *data, size_t count) {
    memcpy((void *)(XIP_BASE + flash_offs), data, count);
}

// ------------------------------------------------------------------------------------------- watchdog

static uint32_t watchdog_load;

void watchdog_enable(uint32_t delay_ms, bool pause_on_debug) {
    (void)pause_on_debug;
    watchdog_load = delay_ms * (SYSCLK_HZ / 1000u);
    WDOG_LOCK = WDOG_UNLOCK_KEY;
    WDOG_LOAD = watchdog_load;
    WDOG_CTRL = WDOG_CTRL_INTEN;
    WDOG_LOCK = 0;
}

void watchdog_update(void) {
    WDOG_LOCK = WDOG_UNLOCK_KEY;
    WDOG_LOAD = watchdog_load; // writing LOAD reloads the counter
    WDOG_LOCK = 0;
}

// The watchdog's first timeout raises its interrupt, which the SSE-200 routes to NMI.
// Reset there so the timeout matches the Pico (one period, not two).
void NMI_Handler(void) {
    static const char msg[] = "\r\n*** watchdog timeout, rebooting ***\r\n";
    _write(1, msg, sizeof(msg) - 1);
    system_reset();
}
