#include "internet_task_if.h"
#include "hekate_utils.h"
#include "free_rtos_memory.h"
#include "sim7020_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "log.h"
#include "semphr.h"

#include "pico/stdlib.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

static set_time_callback_t this_time_callback;

static SemaphoreHandle_t sim_init_done_sem;
static SemaphoreHandle_t sim_mutex;

static TaskHandle_t sim7020_task_handle;
#define SIM7020_TASK_STACK_SIZE_WORDS 1024
static StackType_t sim7020_task_stack[SIM7020_TASK_STACK_SIZE_WORDS];
static StaticTask_t sim7020_task_task_buffer;


// bool internet_task_trigger_get_time(void)
// {
//     static bool sim_initialized = false;

//     if (!sim_initialized)
//     {
//         if (xSemaphoreTake(sim_init_done_sem, pdTICKS_TO_MS(60000)) == pdTRUE)
//         {
//             sim_initialized = true;
//         }
//         else
//         {
//             log_error("failed to take sim_init_done_sem");
//             return false;
//         }
//     }

//     if (xSemaphoreTake(sim_mutex, 1000) == pdTRUE)
//     {
//         get_time_ntp();
//         xSemaphoreGive(sim_mutex);
//     }

//     return true;
// }