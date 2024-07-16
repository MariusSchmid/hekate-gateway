

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

int main()
{

    stdio_init_all();

    gateway_task_init();
    // packet_forwarder_task_init();

    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
    while (1)
    {
        sleep_ms(10);
        printf("FreeRTOS issue");
    }
}