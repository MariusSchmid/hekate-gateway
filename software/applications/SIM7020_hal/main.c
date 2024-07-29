

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "hardware/uart.h"
#include "hardware/irq.h"

#include "tusb.h"

#include "log.h"

#define WAIT_FOR_CDC 1

#include "sim7020_hal.h"
#include "sim7020.h"

char sim_info_buffer[1024];

#define MEMORY_STATS_STACK_SIZE_WORDS 1024
StackType_t memory_stats_task_stack[MEMORY_STATS_STACK_SIZE_WORDS];
StaticTask_t memory_stats_stack_buffer;

void stats_task(void *pvParameters)
{
    if (!sim7020_hal_uart_init())
    {
        log_error("sim7020_hal_uart_init failed");
    }
    if (!sim7020_hal_sim_gpio_init())
    {
        log_error("sim7020_hal_sim_gpio_init failed");
    }
    if (!sim7020_hal_enable_sim_module())
    {
        log_error("sim7020_hal_enable_sim_module failed");
    }
    if (!sim7020_initialize_sim_module())
    {
        log_error("sim7020_initialize_sim_module failed");
    }
    vTaskDelay(pdTICKS_TO_MS(2000));
    uint16_t size;

    while (1)
    {
        sim7020_get_information_json(sim_info_buffer, sizeof(sim_info_buffer), &size);
        log_trace(sim_info_buffer);
        vTaskDelay(pdTICKS_TO_MS(5000));
    }
}

int main()
{

#if 1
    if (!stdio_init_all())
    {
        log_error("failed: stdio_usb_init()");
    }
#endif
#if (0)
    while (!tud_cdc_connected())
    {
        printf(".");
        sleep_ms(500);
    }
#endif
    sim7020_hal_init();
    TaskHandle_t stats_task_handle = xTaskCreateStatic(stats_task,
                                                       "STATS_TASK",
                                                       MEMORY_STATS_STACK_SIZE_WORDS,
                                                       NULL,
                                                       1,
                                                       memory_stats_task_stack,
                                                       &memory_stats_stack_buffer

    );
    if (stats_task_handle == NULL)
    {
        log_error("xTaskCreate failed: wifi_task");
    }
    vTaskStartScheduler();
    while (1)
    {
        sleep_ms(1000);
        log_error("unexpected FreeRTOS Error");
    }
}