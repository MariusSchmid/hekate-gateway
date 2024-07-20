

#include "pico/stdlib.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"



int main()
{

    // stdio_init_all();
    if (!stdio_usb_init())
    {
        log_error("failed: stdio_usb_init()");
    }

#if (WAIT_FOR_CDC == 1)
    while (!tud_cdc_connected())
    {
        printf(".");
        sleep_ms(500);
    }
#endif

    while (true)
    {
        /* code */
    }
    
}