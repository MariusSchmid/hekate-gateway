

#include "gateway_task.h"
#include "packet_forwarder_task.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("vApplicationStackOverflowHook");
}
#include "tusb.h"
int main()
{

    // stdio_init_all();
    stdio_usb_init();

    while (!tud_cdc_connected()) {
      printf(".");
      sleep_ms(500);
    }
    // gateway_task_init();
    packet_forwarder_task_init();

    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
    while (1)
    {
        sleep_ms(10);
        printf("FreeRTOS issue");
    }
}