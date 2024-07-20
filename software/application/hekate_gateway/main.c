

#include "gateway_task.h"
#include "packet_forwarder_task.h"
#include "sim7020_task.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"

#define WAIT_FOR_CDC 0      /* Wait for USB serial connectivity before continue*/
#define ENABLE_GW_TASK 0      /* Start Gatway Task*/
#define ENABLE_SIM7020_TASK 1 /* Start Packet forwarder task*/
#define ENABLE_PKT_FWD 0      /* Start Packet forwarder task*/

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    log_error("vApplicationStackOverflowHook");
}

int main()
{


    if (!stdio_init_all())
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

#if (ENABLE_PKT_FWD == 1)
    packet_forwarder_task_init();
#endif

#if (ENABLE_GW_TASK == 1)
    gateway_task_init();
#endif

#if (ENABLE_SIM7020_TASK == 1)
    sim7020_task_init();
#endif
    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
    while (1)
    {
        sleep_ms(10);
        log_error("unexpected FreeRTOS Error");
    }
}