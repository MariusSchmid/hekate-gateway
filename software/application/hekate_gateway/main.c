

#include "gateway_task.h"
#include "packet_forwarder_task.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("vApplicationStackOverflowHook");
}

int main()
{

    // stdio_init_all();
    stdio_usb_init();

#if 0
    while (!tud_cdc_connected())
    {
        printf(".");
        sleep_ms(500);
    }
#endif

    packet_forwarder_task_init();
    gateway_task_init();

    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
    while (1)
    {
        sleep_ms(10);
        printf("FreeRTOS issue");
    }
}