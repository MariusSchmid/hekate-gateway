#include <stdio.h>
#include "pico/stdlib.h"
#include "concentrator.h"
#include "concentrator_types.h"

#include "hardware/spi.h"

static concentrator_spi_if_t spi_if;

int main() {

    const uint led_pin = 0;

    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    // Initialize chosen serial port
    stdio_init_all();

    // concentrator
    concentrator_init(spi_if);

    // Loop forever
    while (true) {

        // Blink LED
        printf("Blinking!\r\n");
        gpio_put(led_pin, true);
        sleep_ms(1000);
        gpio_put(led_pin, false);
        sleep_ms(1000);
    }
}