#include <hardware/flash.h>
#include <pico/flash.h>
#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef CYW43_WL_GPIO_LED_PIN
#include <pico/cyw43_arch.h>
#endif

#define XIP_OFFSET (256 * 1024)

static void flash_range_erase_callback(void *param)
{
    uint32_t offset = (uint32_t)param;
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
}

static void flash_range_program_callback(void *param)
{
    uint32_t offset = ((uintptr_t*)param)[0];
    const uint8_t *data = (const uint8_t *)((uintptr_t*)param)[1];
    flash_range_program(offset, data, FLASH_PAGE_SIZE);
}

int main()
{
    int status;
    uint8_t write_buffer[FLASH_PAGE_SIZE];
    uint8_t *read_buffer = (uint8_t *)(XIP_BASE + XIP_OFFSET);
    uintptr_t params[] = { XIP_OFFSET, (uintptr_t)write_buffer };
    absolute_time_t next;

    stdio_init_all();
#ifdef CYW43_WL_GPIO_LED_PIN
    status = cyw43_arch_init();
    if (status != PICO_OK) {
        printf("Wi-Fi init failed (this is needed for LED on Pico 2 W)");
        return -1;
    }
#else
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif

    status = flash_safe_execute(flash_range_erase_callback, (void *)XIP_OFFSET, UINT32_MAX);
    if (status != PICO_OK) {
        return -1;
    }
    for (size_t i = 0; i < sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)rand();
    }
    status = flash_safe_execute(flash_range_program_callback, params, UINT32_MAX);
    if (status != PICO_OK) {
        return -1;
    }
    for (size_t i = 0; i < sizeof(write_buffer); i++) {
        if (read_buffer[i] != write_buffer[i]) {
            return -1;
        }
    }

    next = make_timeout_time_ms(10000);
    while (1) {
        if (time_reached(next)) {
            printf("10 second timer went off\r\n");
            next = make_timeout_time_ms(10000);
        }
#ifdef CYW43_WL_GPIO_LED_PIN
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
#else
        gpio_put(PICO_DEFAULT_LED_PIN, true);
#endif
        sleep_ms(1000);
        printf("Hello\r\n");
#ifdef CYW43_WL_GPIO_LED_PIN
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
#else
        gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
        sleep_ms(1000);
        printf("World!\r\n");
    }
}
