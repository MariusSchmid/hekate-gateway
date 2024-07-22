

#include "gateway_task.h"
#include "packet_forwarder_task.h"
#include "internet_task_if.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"

#define WAIT_FOR_CDC 1   /* Wait for USB serial connectivity before continue*/
#define ENABLE_GW_TASK 0 /* Start Gatway Task*/
#define ENABLE_PKT_FWD 1 /* Start Packet forwarder task*/

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    log_error("vApplicationStackOverflowHook");
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   configSTACK_DEPTH_TYPE *puxIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE *puxTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *puxTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
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

#if (INTERNET_WIFI == 1 || INTERNET_SIM)
    internet_task_init();
#endif

#if (ENABLE_PKT_FWD == 1)
    packet_forwarder_task_init();
#endif

#if (ENABLE_GW_TASK == 1)
    gateway_task_init();
#endif

    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
    while (1)
    {
        sleep_ms(10);
        log_error("unexpected FreeRTOS Error");
    }
}