// Minimal stand-in for the Pico SDK's <pico/stdlib.h> when building for the QEMU mps2-an521 machine.
#ifndef QEMU_PICO_STDLIB_H
#define QEMU_PICO_STDLIB_H

#include <pico.h>

// stdio (printf goes out of CMSDK UART0)
bool stdio_init_all(void);

// gpio
#define GPIO_IN false
#define GPIO_OUT true
void gpio_init(unsigned int gpio);
void gpio_set_dir(unsigned int gpio, bool out);
void gpio_put(unsigned int gpio, bool value);

// time
typedef uint64_t absolute_time_t;
uint64_t time_us_64(void);
static inline absolute_time_t get_absolute_time(void) { return time_us_64(); }
static inline absolute_time_t make_timeout_time_us(uint64_t us) { return time_us_64() + us; }
static inline absolute_time_t make_timeout_time_ms(uint32_t ms) { return time_us_64() + ms * 1000ull; }
static inline bool time_reached(absolute_time_t t) { return time_us_64() >= t; }
void sleep_us(uint64_t us);
void sleep_ms(uint32_t ms);

// alarms (a single alarm slot, serviced from the SysTick interrupt)
typedef int32_t alarm_id_t;
typedef int64_t (*alarm_callback_t)(alarm_id_t id, void *user_data);
alarm_id_t add_alarm_in_us(uint64_t us, alarm_callback_t callback, void *user_data, bool fire_if_past);
static inline alarm_id_t add_alarm_in_ms(uint32_t ms, alarm_callback_t callback, void *user_data, bool fire_if_past) {
    return add_alarm_in_us(ms * 1000ull, callback, user_data, fire_if_past);
}

#endif
